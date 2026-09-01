#include <sys.nyt>

int main() {
    string path = "tests/basic/std/io.ny";
    int size = sys::file_size(path);
    if (size < 0) {
        print "Could not get size for: ", path;
    } else {
        print "File: ", path, " Size: ", size, " bytes";
    }
    int fd = sys::open("ghost_file.txt", 0);
    if (fd < 0) {
        int err = sys::last_error();
        print "Open failed as expected. Errno: ", err;
    } else {
        sys::close(fd);
    }
    return 0;
}
