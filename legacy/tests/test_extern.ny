extern int malloc(int size);
extern int free(int ptr);
extern int printf(string format, int value);

int main() {
    int heap_ptr = malloc(20); 
    
    int secret = 1337;
    printf("The secret heap number is: %d", secret);
    
    if (heap_ptr != 0) {
        print(1); // Success: we got an address back!
    } else {
        print(0); // Fail: malloc returned null
    }

    free(heap_ptr);
    return 0;
}
