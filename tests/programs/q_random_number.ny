#include <string.nyt>
#include <sys.nyt>

int main() {
    qubit q1; qubit q2; qubit q3; qubit q4; qubit q5; qubit q6; qubit q7; qubit q8;
    int byte[8];

    h -> q1; byte[0] = (int)q1;
    h -> q2; byte[1] = (int)q2;
    h -> q3; byte[2] = (int)q3;
    h -> q4; byte[3] = (int)q4;
    h -> q5; byte[4] = (int)q5;
    h -> q6; byte[5] = (int)q6;
    h -> q7; byte[6] = (int)q7;
    h -> q8; byte[7] = (int)q8;

    string f = format("{}{}{}{}{}{}" : byte[0] : byte[1] : byte[2] : byte[3]);
    string s = format("{}{}{}{}{}{}" : byte[4] : byte[5] : byte[6] : byte[7]);
    string first = std::ny_str_take(f, 4);
    string second = std::ny_str_take(s, 4);

    string final = std::ny_strcat(first, second);
    print "byte: ", final;

    string cmd = format("perl -le 'print q(int: ), unpack(q(C), pack(q(B8), q({})))'" : final);
    std::sys_exec(cmd);

    return 0;
}
