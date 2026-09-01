#include <sys.nyt>
#include <io.nyt>

int main() {
    string str = "this is a string.";
    std::println("println working!");
    std::puts(str);
    sys::write(2, "stderr\n", 7);
    char buf[128];
    string file = "stress_test.ny";
    int fd = sys::open(file, 2);
    int bytes_read = sys::read(fd, buf, 5000);
    print "bytes read from '", file, "': ", bytes_read;
    if (bytes_read < 0) { return 1; }
    sys::close(fd);
    return 0;
}
