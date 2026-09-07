import os
import subprocess
import glob
import uuid
import shutil
from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

@app.route('/compile', methods=['POST'])
def compile_code():
    data = request.get_json()
    user_code = data.get('code', '')

    root_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    executable = os.path.join(root_dir, "build/bin/nytro")

    job_id = str(uuid.uuid4())[:8]
    job_dir = os.path.join(root_dir, f"job_{job_id}")
    os.makedirs(job_dir, exist_ok=True)

    input_filename = "code.ny"
    input_path = os.path.join(job_dir, input_filename)
    lua_path = os.path.join(job_dir, "init.lua")

    with open(input_path, "w") as f:
        f.write(user_code)

    lua_config = f"""
project = {{
    name = "web_build_{job_id}",
    sources = {{ "{input_filename}" }},
    settings = {{
        verbose = false,
        debug = false,
        clean = true,
        assembler = "nasm",
    }},
}}
"""
    with open(lua_path, "w") as f:
        f.write(lua_config)

    try:
        process = subprocess.run(
            [executable],
            capture_output=True,
            text=True,
            timeout=10,
            cwd=job_dir 
        )

        asm_content = ""
        asm_files = glob.glob(os.path.join(job_dir, "**/*.asm"), recursive=True)

        if asm_files:
            latest_asm = max(asm_files, key=os.path.getmtime)
            with open(latest_asm, "r") as f:
                asm_content = f.read()

        return jsonify({
            "stdout": process.stdout,
            "stderr": process.stderr,
            "asm": asm_content,
            "exit_code": process.returncode
        })

    except Exception as e:
        return jsonify({"stderr": str(e)}), 500

if __name__ == "__main__":
    port = int(os.environ.get("PORT", 8000))
    app.run(host='0.0.0.0', port=port)
