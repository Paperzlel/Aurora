#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

typedef enum {
    PRINTF_STATE_NORMAL,
    PRINTF_STATE_IDENTIFIER,
    PRINTF_STATE_LENGTH_SHORT,
    PRINTF_STATE_LENGTH_LONG,
} PrintfState;

typedef enum {
    LENGTH_SHORT_SHORT,
    LENGTH_SHORT,
    LENGTH_DEFAULT,
    LENGTH_LONG,
    LENGTH_LONG_LONG,
} FmtLength;

static bool a_is_captial = false;
static const char a_hex_lowercase[] = "0123456789abcdef";
static const char a_hex_uppercase[] = "0123456789ABCDEF";

static void print_num_unsigned(uint64_t number, int base) {
    char buf[32]; // Shouldn't be bigger than this
    int pos = 0;

    do {
        uint64_t rem = number % base;
        number /= base;
        buf[pos++] = a_is_captial ? a_hex_uppercase[rem] : a_hex_lowercase[rem];
    } while (number > 0);

    while (--pos >= 0) {
        putc(buf[pos]);
    }
}

static void print_num_signed(int64_t number, int base) {
    if (number < 0) {
        putc('-');
        print_num_unsigned(-number, base);
    } else {
        print_num_unsigned(number, base);
    }
}

void printf(const char *fmt, ...) {
    va_list arg_ptr;
    va_start(arg_ptr, fmt);

    PrintfState state = PRINTF_STATE_NORMAL;
    FmtLength length = LENGTH_DEFAULT;
    int base = 10;
    bool number = false;
    bool is_signed = false;

    while (*fmt) {
        
        switch (state) {
            case PRINTF_STATE_NORMAL: {
                switch (*fmt) {
                    case '%':
                        state = PRINTF_STATE_IDENTIFIER;
                        break;
                    default:
                        putc(*fmt);
                        break;
                }
            } break;

            case PRINTF_STATE_LENGTH_SHORT: {
                if (*fmt == 'h') {
                    length = LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_IDENTIFIER;
                    break;
                } else {
                    goto PRINTF_STATE_IDENTIFIER_;
                }
            } break;


            case PRINTF_STATE_LENGTH_LONG: {
                if (*fmt == 'l') {
                    length = LENGTH_LONG_LONG;
                    state = PRINTF_STATE_IDENTIFIER;
                    break;
                } else {
                    goto PRINTF_STATE_IDENTIFIER_;
                }
            }

            case PRINTF_STATE_IDENTIFIER:
            PRINTF_STATE_IDENTIFIER_:
                switch (*fmt) {
                    case '%':
                        putc(*fmt);
                        state = PRINTF_STATE_NORMAL;
                        break;
                    case 'c':
                        putc((char)va_arg(arg_ptr, int));
                        state = PRINTF_STATE_NORMAL;
                        break;
                    case 's':
                        puts(va_arg(arg_ptr, const char *));
                        state = PRINTF_STATE_NORMAL;
                        break;
                    case 'h':
                        state = PRINTF_STATE_LENGTH_SHORT;
                        fmt++;
                        continue;
                    case 'l':
                        state = PRINTF_STATE_LENGTH_LONG;
                        fmt++;
                        continue;                   // Since we print numbers after this point, increment the pointer and move on immediately
                    case 'd':
                    case 'i':
                        base = 10;
                        is_signed = true;
                        number = true;
                        break;
                    case 'u':
                        base = 10;
                        is_signed = false;
                        number = true;
                        break;
                    case 'x':
                    case 'p':
                        a_is_captial = false;
                        base = 16;
                        is_signed = false;
                        number = true;
                        break;
                    case 'X':
                        a_is_captial = true;
                        base = 16;
                        is_signed = false;
                        number = true;
                        break;
                    case 'o':
                        base = 8;
                        is_signed = false;
                        number = true;
                        break;
                    default:
                        // NOTE: Putting identifiers without an identifier is UB
                        break;      // Ignore undefined types for now (f/F, e/E, g/G, a/A, n)
                }

                if (number) {
                    if (is_signed) {
                        switch (length) {
                            case LENGTH_SHORT_SHORT:
                                print_num_signed((int8_t)va_arg(arg_ptr, int), base);
                                break;
                            case LENGTH_SHORT:
                                print_num_signed((int16_t)va_arg(arg_ptr, int), base);
                                break;
                            case LENGTH_DEFAULT:
                                print_num_signed(va_arg(arg_ptr, int), base);
                                break;
                            case LENGTH_LONG:
                                print_num_signed(va_arg(arg_ptr, long), base);
                                break;
                            case LENGTH_LONG_LONG:
                                print_num_signed(va_arg(arg_ptr, int64_t), base);
                                break;
                        }
                    } else {
                        switch (length) {
                            case LENGTH_SHORT_SHORT:
                                print_num_unsigned((uint8_t)va_arg(arg_ptr, uint32_t), base);
                                break;
                            case LENGTH_SHORT:
                                print_num_unsigned((uint16_t)va_arg(arg_ptr, uint32_t), base);
                                break;
                            case LENGTH_DEFAULT:
                                print_num_unsigned(va_arg(arg_ptr, uint32_t), base);
                                break;
                            case LENGTH_LONG:
                                print_num_unsigned(va_arg(arg_ptr, unsigned long), base);
                                break;
                            case LENGTH_LONG_LONG:
                                print_num_unsigned(va_arg(arg_ptr, uint64_t), base);
                                break;
                        }
                    }
                }

                state = PRINTF_STATE_NORMAL;
                length = LENGTH_DEFAULT;
                base = 10;
                is_signed = false;
                number = false;
                break;
        }

        fmt++;
    }

    va_end(arg_ptr);
}
