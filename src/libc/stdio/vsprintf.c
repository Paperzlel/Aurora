#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

enum PrintState
{
    PRINT_STATE_NORMAL,
    PRINT_STATE_IDENTIFIER,
    PRINT_STATE_LEN_SHORT,
    PRINT_STATE_LEN_LONG,
    PRINT_STATE_SPACE_COUNT,
};

enum FmtLen
{
    LEN_SHORT_SHORT,
    LEN_SHORT,
    LEN_DEFAULT,
    LEN_LONG,
    LEN_LONG_LONG
};

static bool a_is_captial = false;
static bool a_prepend_chars = false;
static bool a_are_zeroes = false;
static uint8_t a_format_count = 0;
static const char a_hex_lowercase[] = "0123456789abcdef";
static const char a_hex_uppercase[] = "0123456789ABCDEF";

static void print_num_unsigned(uint64_t number, int base, char *s, int *count)
{
    char buf[32]; // Shouldn't be bigger than this
    int pos = 0;

    do
    {
        uint64_t rem = number % base;
        number /= base;
        buf[pos++] = a_is_captial ? a_hex_uppercase[rem] : a_hex_lowercase[rem];
    }
    while (number > 0);

    s += *count;
    // Only prepend values if there is space to do so
    if (a_prepend_chars && pos < a_format_count)
    {
        int rem = a_format_count - pos;

        *count += rem;
        while (--rem >= 0)
        {
            *s = a_are_zeroes ? '0' : ' ';
            s++;
        }

    }

    *count += pos;
    while (--pos >= 0)
    {
        *s = buf[pos];
        s++;
    }

}

static void print_num_signed(int64_t number, int base, char *s, int *count)
{
    if (number < 0)
    {
        s[*count] = '-';
        *count += 1;
        print_num_unsigned(-number, base, s, count);
    }
    else
    {
        print_num_unsigned(number, base, s, count);
    }
}

static void print_string(char *s, const char *in, int *count)
{
    s += *count;
    size_t len = strlen(in);
    if (a_prepend_chars && len < a_format_count)
    {
        int rem = a_format_count - len;

        while (--rem >= 0)
        {
            *s = ' ';
            s++;
        }

        *count += rem;
    }

    *count += len;
    while (*in)
    {
        *s = *in;
        s++;
        in++;
    }
}

int vsprintf(char *restrict s, const char *restrict format, va_list args)
{
    int count = 0;
    enum PrintState state = PRINT_STATE_NORMAL;
    enum FmtLen len = LEN_DEFAULT;
    int base = 10;
    bool is_signed = false;
    bool is_num = false;

    while (*format)
    {
        switch (state)
        {
            case PRINT_STATE_NORMAL:
            {
                switch (*format)
                {
                    case '%':
                        state = PRINT_STATE_IDENTIFIER;
                        break;
                    default:
                        s[count] = *format;
                        count++;
                        break;
                }
            } break;

            case PRINT_STATE_LEN_SHORT:
            {
                if (*format == 'h')
                {
                    len = LEN_SHORT_SHORT;
                }

                state = PRINT_STATE_IDENTIFIER;
            } break;


            case PRINT_STATE_LEN_LONG:
            {
                if (*format == 'l')
                {
                    len = LEN_LONG_LONG;
                }

                state = PRINT_STATE_IDENTIFIER;
            } break;

            case PRINT_STATE_SPACE_COUNT:
            {
                a_format_count = *format - 48;
                if (a_format_count == 0 || a_format_count > 9)
                {
                    a_format_count = 0;
                    format--;
                    break;
                }
                
                a_prepend_chars = true;
                state = PRINT_STATE_IDENTIFIER;
            } break;

            case PRINT_STATE_IDENTIFIER:
            {
                switch (*format)
                {
                    case '%':
                        s[count] = *format;
                        state = PRINT_STATE_NORMAL;
                        break;
                    case 'c':
                        s[count] = (char)va_arg(args, int);
                        state = PRINT_STATE_NORMAL;
                        count++;
                        break;
                    case 's':
                        print_string(s, va_arg(args, const char *), &count);
                        state = PRINT_STATE_NORMAL;
                        break;
                    case 'h':
                        state = PRINT_STATE_LEN_SHORT;
                        len = LEN_SHORT;
                        format++;
                        continue;
                    case 'l':
                        state = PRINT_STATE_LEN_LONG;
                        len = LEN_LONG;
                        format++;
                        continue;
                    case 'd':
                    case 'i':
                        base = 10;
                        is_signed = true;
                        is_num = true;
                        break;
                    case 'u':
                        base = 10;
                        is_signed = false;
                        is_num = true;
                        break;
                    case 'x':
                    case 'p':
                        a_is_captial = false;
                        base = 16;
                        is_signed = false;
                        is_num = true;
                        break;
                    case 'X':
                        a_is_captial = true;
                        base = 16;
                        is_signed = false;
                        is_num = true;
                        break;
                    case 'o':
                        base = 8;
                        is_signed = false;
                        is_num = true;
                        break;
                    case '0':
                        state = PRINT_STATE_SPACE_COUNT;
                        a_are_zeroes = true;
                        format++;
                        continue;
                    case '1':
                    case '2':
                    case '3':
                    case '4':
                    case '5':
                    case '6':
                    case '7':
                    case '8':
                    case '9':
                        state = PRINT_STATE_SPACE_COUNT;
                        a_prepend_chars = true;
                        continue;
                    default:
                        // NOTE: Putting identifiers without an identifier is UB
                        break;      // Ignore undefined types for now (f/F, e/E, g/G, a/A, n)
                }

                if (is_num)
                {
                    if (is_signed)
                    {
                        switch (len)
                        {
                            case LEN_SHORT_SHORT:
                                print_num_signed((int8_t)va_arg(args, int), base, s, &count);
                                break;
                            case LEN_SHORT:
                                print_num_signed((int16_t)va_arg(args, int), base, s, &count);
                                break;
                            case LEN_DEFAULT:
                                print_num_signed(va_arg(args, int), base, s, &count);
                                break;
                            case LEN_LONG:
                                print_num_signed(va_arg(args, long), base, s, &count);
                                break;
                            case LEN_LONG_LONG:
                                print_num_signed(va_arg(args, int64_t), base, s, &count);
                                break;
                        }
                    }
                    else 
                    {
                        switch (len)
                        {
                            case LEN_SHORT_SHORT:
                                print_num_unsigned((uint8_t)va_arg(args, uint32_t), base, s, &count);
                                break;
                            case LEN_SHORT:
                                print_num_unsigned((uint16_t)va_arg(args, uint32_t), base, s, &count);
                                break;
                            case LEN_DEFAULT:
                                print_num_unsigned(va_arg(args, uint32_t), base, s, &count);
                                break;
                            case LEN_LONG:
                                print_num_unsigned(va_arg(args, unsigned long), base, s, &count);
                                break;
                            case LEN_LONG_LONG:
                                print_num_unsigned(va_arg(args, uint64_t), base, s, &count);
                                break;
                        }
                    }
                }

                state = PRINT_STATE_NORMAL;
                len = LEN_DEFAULT;
                base = 10;
                is_signed = false;
                is_num = false;
                a_format_count = 0;
                a_prepend_chars = false;
                a_are_zeroes = false;
            } break;
        }

        format++;
    }

    return count;
}
