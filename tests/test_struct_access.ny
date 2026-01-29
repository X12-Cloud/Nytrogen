struct Point {
    public:
        int x;
    private:
        int y;
};

int main() {
    Point p;
    p.x = 10; // This should be allowed
    //p.y = 20; // This should fail if uncommented
    
    print p.x;

    return 0;
}
