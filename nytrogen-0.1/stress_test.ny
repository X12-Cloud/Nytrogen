// 1. Enums
enum Status {
    OFF,
    ON,
    PENDING
}

// 2. Structs
struct Point {
    int x;
    int y;
}

// 3. Global Constants
const int MAX_VALUE = 100;

// 4. Functions with logic and control flow
int calculate_score(int a, int b) {
    int result = 0;
    
    // Comparison and Arithmetic Ops
    if (a > b) {
        if (b != 0) {
            result = (a * 2) + (b / 2);
        }
    } else {
    result = a - b;
    }
    return result;
}

int main() {
    // 5. Variable Declarations & Struct Instantiation
    
    int i = 0;
    int total = 0;

    Point p;
    p.x = 10;
    p.y = 20;

    print p.x;
    print p.y;

    // 6. Loops (Control Flow)
    while (i < 5) {
        total = total + p.x;
        i = i + 1;
    }
    
    // 7. Testing Enums and Logic
    int current_status = ON;
    if (current_status == ON) {
        total = total + MAX_VALUE;
    }
    
    // 8. Function Calls
    int final_result = calculate_score(total, p.y);
    
    // 9. Built-in Print (assuming it takes expressions)
    print final_result; 
    
    // Comparison test
    if (final_result >= 150) {
        print 1; // True
    } else {
        print 0; // False
    }

    return 0;
}
