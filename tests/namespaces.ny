namespace Math {
    int x = 10;
}

int main() {
    print Math::x; // <--- The Parser needs to handle this!
    return 0;
}
