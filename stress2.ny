enum Config {
    MODE_FAST,
    MODE_SECURE,
    MODE_DEBUG
}

const float GRAVITY = 9.81f;
const int MAX_RETRIES = 5;

int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

bool is_valid(int val, int mode) {
    if (mode == MODE_FAST) {
        return val > 0;
    }
    if (mode == MODE_SECURE) {
        if (val > 100) {
            if (val < 500) {
                return true;
            }
        }
        return false;
    }
    return true; 
}

int system_init() {
    return 1;
}

int main() {
    int status = system_init();
    print "System Status:";
    print status;

    // Skip the loop for now, just test a single access
    int row_map[3];
    int values[9];
    row_map[1] = 3;
    values[4] = 88;

    int r = 1;
    int c = 1;
    print "Accessing simulated 2D array (88):";
    print values[row_map[r] + c];

    // Testing Recursion
    int fact_result = factorial(5);
    print "Factorial of 5 (120):";
    print fact_result;

    // Testing Floats
    float velocity = 10.5f;
    float time = 2.0f;
    float distance = (velocity * time) + (0.5f * GRAVITY * (time * time));
    print "Calculated Distance:";
    print distance;

    // Testing Logic
    int my_val = 250;
    int current_mode = MODE_SECURE;

    if (is_valid(my_val, current_mode)) {
        if (fact_result > 100) {
            print "Validation Logic: SUCCESS";
        }
    }

    return 0;
}
