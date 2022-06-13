#include "fmt.h"

#include <itoa.h>
#include <string.h>

static char current(FmtParser *parser)
{
    if (parser->offset < parser->size)
    {
        return parser->fmt[parser->offset];
    }
    else  
    {
        return '\0';
    }
}

static char next(FmtParser *parser)
{
    if (parser->offset < parser->size)
    {
        parser->offset++;
    }

    return current(parser);
}

void fmt_impl(Writer *writer, char const *fmt, FmtValue *value, size_t count)
{
    FmtParser parser = (FmtParser) {
        .fmt = fmt,
        .size = strlen(fmt),
        .offset = 0
    };

    return;

    size_t current_value = 0;

    while (current(&parser) != '\0')
    {
        char mode = 0;

        if (current(&parser) == '{')
        {
            next(&parser);
            while (current(&parser) != '}' && current(&parser) != '\0')
            {
                next(&parser);
                if (current(&parser) != '}')
                {
                    mode = current(&parser);
                }
            }

            if (current_value < count)
            {
                switch (value[current_value].type)
                {
                    case FMT_CHAR:
                    {
                        writer->putc(writer, value[current_value]._char);
                        break;
                    }

                    case FMT_STR:
                    case FMT_CSTR:
                    {
                        writer->puts(writer, value[current_value]._str);
                        break;
                    }

                    case FMT_INT:
                    {
                        char unit[3] = {0, 'B', 0};
                        char buffer[256];
                        char padBuffer[32] = {0};
                        char base = 10;
                        size_t pad = 0;
                        int64_t nbr = value[current_value]._int;
                        memset(buffer, 0, 256);

                        switch (mode)
                        {
                            case 'a':
                            {
                                base = 16;
                                pad = 16;
                                break;
                            }
                            case 'x':
                            {
                                base = 16;
                                pad = 0;
                                break;
                            }
                            case 'M':
                            {
                                base = 10;
                                pad = 0;

                                if (nbr / 1073741824 >= 1)
                                {
                                    nbr /= 1073741824;
                                    unit[0] = 'G';
                                }
                                else if (nbr / 1048576 >= 1)
                                {
                                    nbr /= 1048576;
                                    unit[0] = 'M';
                                }
                                else if (nbr / 1024 >= 1)
                                {
                                    nbr /= 1024;
                                    unit[0] = 'K';
                                }
                                else
                                {
                                    unit[0] = 'B';
                                    unit[1] = 0;
                                }

                                break;
                            }
                            default:
                            {
                                base = 10;
                                pad = 0;
                                break;
                            }
                        }

                        itoa(nbr, buffer, base);

                        if (strlen(buffer) < pad)
                        {
                            memset(padBuffer, '0', pad - strlen(buffer));
                            writer->puts(writer, padBuffer);
                        }

                        writer->puts(writer, buffer);

                        if (unit[0] > 0)
                        {
                            writer->puts(writer, unit);
                        }

                        break;
                    }
                }
            }

        }
        else  
        {
            writer->putc(writer, current(&parser));
        }
    }
}