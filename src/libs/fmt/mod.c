#include <stdint.h>
#include <string.h>

#include "mod.h"

static char *strrev(char *str)
{
    int start;
    int end;
    char tmp;

    end = strlen(str) - 1;
    start = 0;

    while (start < end)
    {
        tmp = str[start];
        str[start] = str[end];
        str[end] = tmp;
        start++;
        end--;
    }
    return str;
}

static char *itoa(int64_t value, char *str, int base)
{
    int i = 0;
    bool isNegative = false;

    if (value == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    if (value < 0 && base == 10)
    {
        isNegative = true;
        value = -value;
    }

    while (value != 0)
    {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value = value / base;
    }

    if (isNegative)
    {
        str[i++] = '-';
    }

    str[i] = '\0';

    return strrev(str);
}

char *utoa(uint64_t value, char *str, int base)
{
    int i = 0;

    if (value == 0)
    {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    while (value != 0)
    {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value = value / base;
    }

    str[i] = '\0';

    return strrev(str);
}

Res fmt(Stream stream, char const fmt[static 1], ...)
{
    va_list args;
    va_start(args, fmt);
    Res res = vfmt(stream, fmt, args);
    va_end(args);
    return res;
}

Res vfmt(Stream stream, char const fmt[static 1], va_list args)
{
    char *s = (char *)fmt;

    while (*s)
    {
        if (*s == '%')
        {
            switch (*++s)
            {
                case 'd':
                {
                    s++;
                    char buf[100];
                    int64_t value = va_arg(args, int64_t);

                    itoa(value, buf, 10);
                    stream.write(strlen(buf), buf);
                    break;
                }

                case 'u':
                {
                    s++;
                    char buf[100];
                    uint64_t value = va_arg(args, int64_t);

                    utoa(value, buf, 10);
                    stream.write(strlen(buf), buf);
                    break;
                }

                case 'p':
                {
                    s++;
                    char buf[100];
                    uint64_t value = va_arg(args, uint64_t);

                    utoa(value, buf, 16);
                    stream.write(2, "0x");

                    for (size_t i = 0; i < 16 - strlen(buf); i++)
                    {
                        stream.write(1, "0");
                    }

                    stream.write(strlen(buf), buf);
                    break;
                }

                case 'x':
                {
                    s++;
                    char buf[100];
                    uint64_t value = va_arg(args, uint64_t);

                    utoa(value, buf, 16);
                    stream.write(strlen(buf), buf);
                    break;
                }

                case 's':
                {
                    s++;
                    char *value = va_arg(args, char *);
                    stream.write(strlen(value), value);
                    break;
                }

                case 'c':
                {
                    s++;
                    char value = va_arg(args, int);
                    stream.write(1, &value);
                    break;
                }

                case '%':
                {
                    s++;
                    stream.write(1, "%");
                    break;
                }

                case 'z':
                {
                    *s = '%';
                    break;
                }

                default:
                {
                    return err$(RES_INVAL);
                }
            }
        }
        else
        {
            char sub[2] = {*s++, '\0'};
            stream.write(1, sub);
        }
    }

    return uok$(0);
}
