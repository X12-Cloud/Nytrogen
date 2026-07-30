int main() {
    // Float to Int (Truncation Test)
    float f = 42.99f;
    int f_to_i = (int)f; 
    print "Float 42.99 to Int (42): ", f_to_i;

    // Int to Float (Promotion Test)
    int i = 100;
    float i_to_f = (float)i;
    print "Int 100 to Float (100.000000): ", i_to_f;

    // Double to Float (Precision Narrowing)
    double d = 3.1415926535;
    float d_to_f = (float)d;
    print "Double PI to Float (3.141593): ", d_to_f;

    // Int to Char (Narrowing Test)
    int big_i = 65; // ASCII for 'A'
    char i_to_c = (char)big_i;
    print "Int 65 to Char (A): ", i_to_c;

    // Char to Int (Widening Test)
    char c = 'B'; // ASCII 66
    int c_to_i = (int)c;
    print "Char B to Int (66): ", c_to_i;

    // Boolean Conversions
    int truthy = 1;
    bool b = (bool)truthy;
    print "Int 1 to Bool (1): ", b;

    int falsey = 0;
    bool b2 = (bool)falsey;
    print "Int 0 to Bool (0): ", b2;

    // Multi-step Complex Cast
    // Double -> Int -> Float
    double val = 55.55;
    float result = (float)((int)val);
    print "Double 55.55 -> Int -> Float (55.000000): ", result;

    return 0;
}
