#include <math/log.nyt>
#include <debug.nyt>

int main() {
    double x = __builtin_sqrt(4.0);
    double y = __builtin_abs(-10.0);
    double z = __builtin_round(3.14159);

    print x;
    print y;
    print z;

    //int m = math::ln(2.718281828459045);
    //print m;

    std::dbg("hello this is a red debug message", "red");

    return 0;
}
