int main() {
    // Test complex number addition and multiplication
    complex a = 1.0+2.0i;
    complex b = 3.0+4.0i;
    complex c = a + b;

    // Rotation test (Hadamard-like normalization)
    double norm = 0.707106;
    complex h_state = norm + 0.0i;

    print "Complex Add (4+6i): ", c;
    print "State: ", h_state;

    // Logic failsafe
    if (c != 4.0+6.0i) {
        print "Failsafe: Complex math incorrect!";
        return 1;
    }
    return 0;
}
