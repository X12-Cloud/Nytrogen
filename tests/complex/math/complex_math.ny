int main() {
    // Test complex number addition and subtraction
    complex a = 1.0+2.0i;
    complex b = 3.0+4.0i;
    complex c = a + b;
    complex d = a - b;

    // Rotation test (Hadamard-like normalization)
    double norm = 0.707106;
    complex h_state = (complex)norm + 0.0i;

    print "Complex Add (4+6i): ", c;
    print "Complex Sub (-2-2i): ", d;
    print "State: ", h_state;

    // Logic failsafe
    if (c != 4.0+6.0i) {
        print "Failsafe: Complex math incorrect!";
        return 1;
    }
    if (d != (-2.0-2.0i)) {
        print "Failsafe: Complex subtraction incorrect!";
        return 1;
    }

    return 0;
}
