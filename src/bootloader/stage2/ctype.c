#include "ctype.h"

char tolower(char p_in) {
    return ((p_in >= 'A' && p_in <= 'Z') ? p_in + 32 : p_in);
}

char toupper(char p_in) {
    return ((p_in >= 'a' && p_in <= 'z') ? p_in - 32 : p_in);
}