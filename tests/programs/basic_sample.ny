#include <io.nyt>
#include <fs.nyt>
#include <sys.nyt>

int main() {
    std::println("Enter file name: ");
    string file = std::readln();

    if (file == "") {
        file = "init.lua";
        print format "No file name provided defaulting to {}" : file;
    }

    if (fs::exists(file) == 1) {
        int size = std::sys_fsize(file);
        print format "Reading file: {} ({} bytes)\n" : file : size;
        string data = fs::read_all(file);
        print "------------ File data ------------";
        print data, "-----------------------------------\n";
    }

    complex: a = 1.0+1.0i, b = 1.0+2.0i;
    print "Phase Rotation (2+3i): ", a + b;

    qubit q;
    h -> q;
    print "Quantum State (Superposition):\n", q;

    return 0;
}
