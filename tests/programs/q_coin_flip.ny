int main() {
    qubit q;
    int heads = 0;
    int tails = 0;
    int i = 0;
    int total_flips = 100;
    int result = 0;

    print "--- Starting Quantum Statistics Test ---";
    print "Running 100 flips in superposition...", "\n";

    while (i < total_flips) {
        // Ensure qubit is in |0>
        // (We don't have an explicit 'init' gate yet, but _start
        //  clears the vector, and we can use X gates to reset if needed.
        //  For now, we'll just keep applying H).

        h -> q; // Put into superposition

        // Measure the qubit
        result = (int)q;

        if (result == 1) {
            tails = tails + 1;
        } else {
            heads = heads + 1;
        }

        // Reset the qubit for the next flip
        if (result == 1) {
            x -> q;
        }

        i = i + 1;
    }

    print "--- Results ---";
    print "Heads (|0>): ", heads;
    print "Tails (|1>): ", tails;
    print "Total:       ", i;

    // A little logic test
    if (heads > 30) {
        if (tails > 30) {
            print "Status: PASS (Probability distribution is balanced)";
        }
    }

    return 0;
}
