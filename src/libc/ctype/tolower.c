#include <ctype.h>

int tolower(int p_in)
{
    return ((p_in >= 'A' && p_in <= 'Z') ? p_in + 32 : p_in);
}
