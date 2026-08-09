enum Color { RED, GREEN, BLUE };

struct Point {
    int x;
    int y;
    int c;
};

int main() {
    Point p;
    p.x = 10;
    p.y = 20;
    p.c = GREEN;

    print "Point X: ", p.x;
    print "Point Y: ", p.y;
    print "Color (Expected 1): ", (int)p.c;

    if (p.x != 10) { return 1; }
    if ((int)p.c != 1) { return 1; }
    return 0;
}
