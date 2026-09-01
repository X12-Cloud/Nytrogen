#include <math.h>
#include <immintrin.h>

__m128d ny_complex_mul(__m128d a, __m128d b) {
    // a = [imag_a, real_a], b = [imag_b, real_b]
    double ra = ((double*)&a)[0];
    double ia = ((double*)&a)[1];
    double rb = ((double*)&b)[0];
    double ib = ((double*)&b)[1];

    __m128d res;
    ((double*)&res)[0] = ra * rb - ia * ib; // New Real
    ((double*)&res)[1] = ra * ib + ia * rb; // New Imaginary
    return res;
}
