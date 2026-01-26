#include <ctype.h>

int toupper(int p_in)
{
    // TODO: Depends on locale. Fix in the future.
    return ((p_in >= 'a' && p_in <= 'z') ? p_in - 32 : p_in);
}
