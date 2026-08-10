enum Status {
    OFF,
    ON,
    PENDING
};

struct Point {
    int x;
    int y;
};

float PI = 3.14159f;

int calculate_score(int a, int b) {
    print "DEBUG calculate_score: a=", a, ", b=", b;
    if (a > b) {
        return (a * 2) + (b / 2); // 1499 * 2 + 20 / 2
    }
    return a - b;
}

int main() {
    print "Hello", "\n";
    // Array Setup
    int map[3];
    int data[3];

    map[0] = 2;
    map[1] = 0;
    map[2] = 1;

    data[0] = 100;
    data[1] = 200;
    data[2] = 300;

    // Recursive Array Test: data[map[0]]
    // map[0] is 2, so this prints data[2] (300)
    print "nested index access (300): ", data[map[0]];

    // Nested Assignment Test
    // data[map[1]] is data[0]. Setting it to 999.
    data[map[1]] = 999;
    print "nested assignment result (999): ", data[0], "\n";

    // Struct & Enum Logic
    Point p;
    p.x = 10;
    p.y = 20;
    print "- DEBUG -";
    print "p.x | p.y";
    print p.x, "  |  ", p.y, "\n";

    int current_status = ON;
    int total = 0;

    // Loops with Array Access
    int i = 0;
    while (i < 3) {
        print "processing index..."; // Deduplication test
        total = total + data[i]; // 1499
        i = i + 1;
    }
    print "processing index..."; // Should reuse same label
    print "DEBUG total before calculate_score(): ", total;

    // Float Math with Constants
    float radius = 5.0f;
    float radiusp2 = radius * radius;
    float area = PI * radiusp2;
    print "circle area: ", area, "\n";

    // Final Function & Logic check
    int final_result = calculate_score(total, p.y); // total = 1499, p.y = 20. score = 3008
    if (final_result != 0) {
        print "final score (3008): ", final_result;
    }

    if (current_status == ON) {
        if (final_result > 1000) {
            if (final_result == 3008) {
                print "status: pass (3008)";
            } else {
                print "status: pass (not 3008)";
            }
        } else {
            print "status: fail";
            if (final_result == 0) {
                print "reason: final score = 0";
            } else {
                print "reason: final score < 1000";
            }
        }
    }

    // TODO:
    //print format("x = {}, y = {}" :x:y);

    return 0;
}
