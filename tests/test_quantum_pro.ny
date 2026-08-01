int main() {
    // Test Custom Qubit Initialization
    // Initializing q1 to the pure |1> state manually
    // (Normally done by init -> q1; x -> q1;)
    print "--- Testing Custom Qubit Init ---";
    qubit q1 {
        alpha: 0.0+0.0i,
        beta: 1.0+0.0i
    };

    // Print raw amplitudes to see if vmovupd worked
    print q1; 

    // Test Measurement of the manual state
    // Since it is 100% |1>, result MUST be 1.
    int result = (int)q1;
    print "Measured |1> state (should be 1): ", result, "\n";

    // Test Complex Literal Storage
    // This tests if the CodeGen puts these in .data correctly
    print "--- Testing Complex Literals ---";
    complex c1 = 0.707+0.707i;

    print c1;
    print "Complex literal assigned successfully.", "\n";

    print "--- Testing exit(0) Intrinsic ---";
    print "This is the last thing you should see.";

    exit(0);

    print "ERROR: exit() failed! You shouldn't see this.";

    return 0;
}
