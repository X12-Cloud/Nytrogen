include "std/sys/sys_write.nyt";

int main() {
    string msg = "Hello from raw syscall!\n";
    sys_write(1, msg, 24);
}
