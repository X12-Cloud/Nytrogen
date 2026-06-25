#include <iostream>

namespace math {

namespace constants {
    double E = 2.718281828459045235;
} // namespace constants

double round(double x) {
    if (x >= 0.0) {
        return (double)(int)(x + 0.5);
    } else {
        return (double)(int)(x - 0.5);
    }
}

namespace helpers {

bool is_negative(double x) {
    x = round(x); 
    return x < 0.0;
}

bool is_odd(double x) {
    x = round(x);
    long long integer_x = (long long)x;
    return integer_x % 2 != 0;
}

} // namespace helpers

double abs(double x) {
    if (math::helpers::is_negative(x)) {
        x *= -1;
    }
    return x;
}

double sqrt(double num) {
    double result;

    // Check for negative input to prevent CPU domain errors
    if (num < 0.0) {
        return 1;
    }

    asm volatile (
        "sqrtsd %1, %0"     // %1 is the input (num), %0 is the output (result)
        : "=x" (result)     // Output operand: "x" specifies an SSE/XMM register
        : "x" (num)         // Input operand: "x" specifies an SSE/XMM register
    );

    return result;
}

double log2(double x) {
    if (x <= 0.0) return 0.0;

    double result = 0.0;

    // Shift the number into the range [1.0, 2.0) by manipulating the whole integer part
    while (x >= 2.0) {
        result += 1.0;
        x /= 2.0;
    }
    while (x < 1.0) {
        result -= 1.0;
        x *= 2.0;
    }

    // Extract the fractional bits using square roots
    double base_root = 2.0;
    double bit_value = 0.5;

    // Run 52 times because a 64-bit double has exactly 52 bits of mantissa precision
    for (int i = 0; i < 52; ++i) {
        base_root = math::sqrt(base_root);

        if (x >= base_root) {
            result += bit_value;
            x /= base_root; // Scale x down
        }
        bit_value *= 0.5;
    }

    return result;
}

double ln(double x) {
    return math::log2(x) * 0.6931471805599453;
}

double log(double x, double n) {
    if (x <= 0.0 || n <= 0.0 || n == 1.0) {
        return 0.0;
    }

    return math::ln(x)/math::ln(n);
}

double exp(double x) {
    // Edge case protections
    if (x == 0.0) return 1.0;
    if (x < -700.0) return 0.0;  // Underflow: e^-700 is practically 0
    if (x > 700.0) return 1.0/0.0; // Overflow: exceeds double capacity (Infinity)

    // Convert natural exponent to a base-2 exponent
    double p = x * math::log2(math::constants::E);

    // Split p into a whole integer and a fractional remainder
    int integer_part = (int)math::round(p);
    double fraction_part = p - (double)integer_part;

    // Calculate 2^fraction_part using a highly precise polynomial approximation
    double c = fraction_part * math::ln(2.0);
    double fraction_result = 1.0 + c * (1.0 + c * (0.5 + c * (0.16666666666666667 + c * 0.041666666666666664)));

    // Scale the result by 2^integer_part using a basic multiplication/division loop
    double scale = 1.0;
    if (integer_part > 0) {
        for (int i = 0; i < integer_part; ++i) {
            scale *= 2.0;
        }
    } else if (integer_part < 0) {
        for (int i = 0; i < -integer_part; ++i) {
            scale /= 2.0;
        }
    }

    // Combine them
    return fraction_result * scale;
}

double pow(double base, double exp) {
    double result = math::exp(exp * ln(base));
    return result;
}

double grt(double base, double root_power) {
    if (base == 0.0) return 0.0;
    if (root_power == 0.0) return 0.0; // 0th root is undefined

    bool is_root_odd = math::helpers::is_odd(root_power);
    bool is_base_negative = math::helpers::is_negative(base);
    bool is_root_negative = math::helpers::is_negative(root_power);

    double working_root = math::abs(root_power);

    // Handle negative bases
    if (is_base_negative) {
        if (is_root_odd) {
            base = math::abs(base);
        } else {
            std::cerr << "Getting even root of a negative base is not yet supported." << std::endl;
            exit(1);
        }
    }

    // grt(x, y) = e ^ ln(x) / y
    double exponent = ln(base)/working_root;
    double result = math::exp(exponent);

    // Get the reciprocal of the result if root is negative
    if (is_root_negative) {
        result = 1.0 / result;
    }

    // Put the negative sign back in if root was odd
    if (is_root_odd && is_base_negative) {
        return  result * -1;
    } else {
        return result;
    }
}

} // namespace math
