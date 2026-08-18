#include <string.h>
#include <stdlib.h>

int ny_strlen(const char* s) {
    return (int)strlen(s);
}

// string - int (Remove n characters from the end)
char* ny_str_sub_int(const char* s, int n) {
    int len = strlen(s);
    int new_len = (n >= len) ? 0 : len - n;
    char* res = malloc(new_len + 1);
    memcpy(res, s, new_len);
    res[new_len] = '\0';
    return res;
}

// string - string (Remove first occurrence of sub from s)
char* ny_str_sub_str(const char* s, const char* sub) {
    const char* p = strstr(s, sub);
    if (!p) return strdup(s);

    int sub_len = strlen(sub);
    int s_len = strlen(s);
    char* res = malloc(s_len - sub_len + 1);

    int prefix_len = p - s;
    memcpy(res, s, prefix_len); // Copy part before sub
    strcpy(res + prefix_len, p + sub_len); // Copy part after sub

    return res;
}
