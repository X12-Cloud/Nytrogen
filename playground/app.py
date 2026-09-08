import os
import subprocess
import glob
import uuid
import shutil
from flask import Flask, request, jsonify
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

# --- Path Configuration ---
ROOT_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
JOBS_BASE_DIR = os.path.join(ROOT_DIR, "playground", "tmp_jobs")
RUNTIME_HEADERS = os.path.join(ROOT_DIR, "runtime", "headers")

os.makedirs(JOBS_BASE_DIR, exist_ok=True)

@app.route('/compile', methods=['POST'])
def compile_code():
    data = request.get_json()
    user_code = data.get('code', '')
    user_stdin = data.get('stdin', '')
    s = data.get('settings', {})

    job_id = str(uuid.uuid4())[:8]
    job_dir = os.path.join(JOBS_BASE_DIR, f"job_{job_id}")
    os.makedirs(job_dir, exist_ok=True)

    input_filename = "code.ny"
    input_path = os.path.join(job_dir, input_filename)
    lua_path = os.path.join(job_dir, "init.lua")
    executable = os.path.join(ROOT_DIR, "build/bin/nytro")

    with open(input_path, "w") as f:
        f.write(user_code)

    def lua_bool(val): return "true" if val else "false"

    lua_config = f"""
project = {{
    name = "web_build_{job_id}",
    sources = {{ "{input_path}" }},
    settings = {{
        verbose = {lua_bool(s.get('verbose', False))},
        debug = {lua_bool(s.get('debug', False))},
        clean = {lua_bool(s.get('clean', True))},
        assembler = "{s.get('assembler', 'nasm')}",
        optimization = "{s.get('opt', 'O0')}"
    }},
}}
"""
    with open(lua_path, "w") as f:
        f.write(lua_config)

    try:
        env = os.environ.copy()
        env["NYTRO_STDLIB_PATH"] = RUNTIME_HEADERS

        process = subprocess.run(
            [executable],
            input=user_stdin,
            capture_output=True,
            text=True,
            timeout=10,
            cwd=job_dir,
            env=env
        )

        asm_content = ""
        asm_search_path = os.path.join(job_dir, "**/*.asm")
        asm_files = glob.glob(asm_search_path, recursive=True)

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

    except subprocess.TimeoutExpired:
        return jsonify({"stderr": "Timeout: Program took too long to run (10s limit)"}), 408
    except Exception as e:
        return jsonify({"stderr": str(e)}), 500
    finally:
        if os.path.exists(job_dir):
            shutil.rmtree(job_dir)

if __name__ == "__main__":
    port = int(os.environ.get("PORT", 8000))
    app.run(host='0.0.0.0', port=port)
