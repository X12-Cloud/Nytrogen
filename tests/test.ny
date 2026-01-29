include <stdlib.nyt>

int main() {
    char my_char;
    my_char = 'A'; // Assign a character literal
    
    // Call sys_write to print 'my_char' to stdout (fd 1)
    sys_write(1, &my_char, 1);

    return 0;
}