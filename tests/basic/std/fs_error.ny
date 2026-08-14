#include <sys.nyt>

int main() {
    string path = "tests/basic/std/io.ny";
    int size = std::sys_fsize(path);
    if (size < 0) {
        print "Could not get size for: ", path;
    } else {
        print "File: ", path, " Size: ", size, " bytes";
    }
    int fd = std::sys_open("ghost_file.txt", 0); // O_RDONLY
    if (fd < 0) {
        int err = std::sys_last_error();
        print "Open failed as expected. Errno: ", err;
    } else {
        std::sys_close(fd);
    }
    return 0;
}
