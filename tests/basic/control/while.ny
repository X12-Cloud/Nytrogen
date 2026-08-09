int main() {
    int i = 0;
    while (i < 3) {
        int scoped_var = i + 10;
        print "Loop i: ", i, " Scoped: ", scoped_var;
        i = i + 1;
    }
    return 0;
}
