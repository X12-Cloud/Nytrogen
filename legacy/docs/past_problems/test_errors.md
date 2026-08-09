# Errors found while running tests/test_everything.nyt

## 1. Compiler Segmentation Fault
The compiler crashes with a segmentation fault when processing `tests/test_everything.nyt`. This is the most critical error and needs to be addressed first.

## 2. Assembly Errors
The following errors were reported by `nasm` during the assembly phase:

### a. Undefined Symbol
- **Error:** `symbol 'add.main_epilogue' not defined`
- **Cause:** The code generator is incorrectly generating a jump to a label that is local to the `main` function from within another function (`add`). The epilogue label for each function should be local to that function.

### b. Redefined Labels
The following labels are being redefined during code generation:
- `main`
- `main.main_epilogue`
- `_start`
- **Cause:** This is likely due to the preprocessor including multiple files that all define these labels. The code generator should be able to handle this, or the preprocessor should be modified to avoid this.

## 3. Linker Error
- **Error:** `ld: cannot find /run/media/mohamed/Mohamed/Mohameds place/X.co/Nytrogen - Compiler/Nytro-0.1/out/out.o: No such file or directory`
- **Cause:** This error is a direct result of the assembly errors. Since `nasm` fails, the object file `out.o` is not created, and therefore the linker cannot find it.

## 4. Execution Error
- **Error:** `./run.sh: line 76: /run/media/mohamed/Mohamed/Mohameds place/X.co/Nytrogen - Compiler/Nytro-0.1/out/out: No such file or directory`
- **Cause:** This error is a result of the linker error. Since the linker fails, the final executable `out` is not created.
