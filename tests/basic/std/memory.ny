#include <sys.nyt>

int main() {
    char* map = sys::mmap("0", 4096, 3, 34, -1, 0);
    if (map == 0) {
        print "mmap failed!";
        return 1;
    }
    print "Successfully mapped 4096 bytes of memory.";
    return 0;
}
