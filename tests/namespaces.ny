namespace math {
    int x = 10;
}

int main() {
    math::x = 20; // This will trigger the assignment code
    int y = math::x;
    print y;
    return 0;
}
