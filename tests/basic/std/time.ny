#include <sys.nyt>

int main() {
    print "Starting loop benchmark...";
    int start = sys::get_nanos();
    int i = 0;
    while (i < 1000000) {
        i = i + 1;
    }
    int end = sys::get_nanos();
    int diff = end - start;
    print "Loop took: ", diff, " nanoseconds.";
    return 0;
}
