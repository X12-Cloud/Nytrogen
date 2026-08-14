#include <sys.nyt>
#include <io.nyt>

int main() {
    string str = "this is a string.";
    std::println("println working!");
    std::put(str);
    std::sys_write(2, "stderr\n", 7);
    char buf[128];
    string file = "stress_test.ny";
    int fd = std::sys_open(file, 2);
    int bytes_read = std::sys_read(fd, buf, 5000);
    print "bytes read from '", file, "': ", bytes_read;
    if (bytes_read < 0) { return 1; }
    //std::sys_write(fd, "HELLO THERE MATE", 16);
    std::sys_close(fd);
    return 0;
}
