#include <math.nyt>

int main() {
    double val = -16.0;
    double positive = math::abs(val);
    double root = math::sqrt(positive);
    double decimal = 4.7;
    int rounded = (int)math::round(decimal);

    print "abs(-16): ", positive;
    print "sqrt(16): ", root;
    print "round(4.7): ", rounded;

    if (root != 4.0) { return 1; }
    if (rounded != 5) { return 1; }
    return 0;
}
