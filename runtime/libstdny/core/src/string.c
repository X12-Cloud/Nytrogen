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

// string + string (Concatenate two strings)
char* ny_strcat(const char* s1, const char* s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    char* res = malloc(len1 + len2 + 1);
    memcpy(res, s1, len1);
    memcpy(res + len1, s2, len2 + 1); // includes null terminator
    return res;
}

// Take the first N characters of a string
char* ny_str_take(const char* s, int n) {
    int len = strlen(s);
    int take_len = (n < 0) ? 0 : (n > len ? len : n);
    char* res = malloc(take_len + 1);
    memcpy(res, s, take_len);
    res[take_len] = '\0';
    return res;
}
