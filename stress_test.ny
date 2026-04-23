extern int hi();

enum Status {
    OFF,
    ON,
    PENDING
}

struct Point {
    int x;
    int y;
};

const float PI = 3.14159f;

int calculate_score(int a, int b) {
    if (a > b) {
        return (a * 2) + (b / 2);
    }
    return a - b;
}

int main() {
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
    print "nested index access (300):";
    print data[map[0]];

    // Nested Assignment Test
    // data[map[1]] is data[0]. Setting it to 999.
    data[map[1]] = 999;
    print "nested assignment result (999):";
    print data[0];

    // Struct & Enum Logic
    Point p;
    p.x = 10;
    p.y = 20;
    
    int current_status = ON;
    int total = 0;

    // Loops with Array Access
    int i = 0;
    while (i < 3) {
        print "processing index..."; // Deduplication test
        total = total + data[i];
        i = i + 1;
    }
    print "processing index..."; // Should reuse same label

    // Float Math with Constants
    float radius = 5.0f;
    float area = PI * (radius * radius);
    print "circle area:";
    print area;

    // Final Function & Logic check
    int final_result = calculate_score(total, p.y);
    print "final score:";
    print final_result;

    if (current_status == ON) {
        if (final_result > 1000) {
            print "status: pass";
        } else {
            print "status: fail";
        }
    }

    hi();

    return 0;
}
