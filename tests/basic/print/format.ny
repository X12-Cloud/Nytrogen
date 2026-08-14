int main() {
    string name = "x12";
    int n = 21;
    string msg = format "Hello {}, the number is {}!" : name : n;
    print msg;
    return 0;
}
