namespace ns {
    int x = 10;

    int add(int a, int b) {
        return a + b;
    }
}

int main() {
    print "ns::x = ", ns::x;
    int x = 2;
    x = ns::add(1, 5);
    print x;
    return 0;
}
