int main() {
    qubit q;
    int heads = 0;
    int tails = 0;
    int i = 0;
    int total_flips = 100;

    print "--- Starting Quantum Statistics Test ---";
    print "Running 100 flips in superposition...", "\n";

    while (i < total_flips) {
        // 1. Ensure qubit is in |0>
        // (We don't have an explicit 'init' gate yet, but your _start 
        //  clears the vector, and we can use X gates to reset if needed. 
        //  For now, we'll just keep applying H).
        
        h -> q; // Put into 50/50 superposition

        // 2. Measure the qubit by casting to int
        int result = (int)q;

        if (result == 1) {
            tails = tails + 1;
        } else {
            heads = heads + 1;
        }

        // 3. Reset the qubit for the next flip
        // If it collapsed to |1>, flip it back to |0>
        if (result == 1) {
            x -> q;
        }

        i = i + 1;
    }

    print "--- Results ---";
    print "Heads (|0>): ", heads;
    print "Tails (|1>): ", tails;
    print "Total:       ", i;

    // A little logic test:
    if (heads > 30) {
        if (tails > 30) {
            print "Status: PASS (Probability distribution is balanced)";
        }
    }

    return 0;
}
