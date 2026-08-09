int factorial(int n) {
    if (n == 0) { return 1; }
    return n * factorial(n - 1);
}

int main() {
    print "Factorial of 5 (120): ", factorial(5);
    return 0;
}
