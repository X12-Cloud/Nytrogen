#include <io.nyt>

int main() {
    int: x = 10, y = 20, z = x + y;
    string a = "hello";
    bool b = true;
    char c = 'm';
    print z, " ", a, " ", b, " ", c; // 30 hello true m
    string err = "ERROR";
    print err to std::err;
    std::println("hello from println");
    return 0;
}
