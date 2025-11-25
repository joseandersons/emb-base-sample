#include "plate_validator.h"
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

static bool is_letter(char c)
{
    c = (char)toupper((unsigned char)c);
    return (c >= 'A' && c <= 'Z');
}

static bool is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

static bool match_brasil(const char *p)
{
    return is_letter(p[0]) &&
           is_letter(p[1]) &&
           is_letter(p[2]) &&
           is_digit (p[3]) &&
           is_letter(p[4]) &&
           is_digit (p[5]) &&
           is_digit (p[6]);
}

static bool match_arg_ven(const char *p)
{
    return is_letter(p[0]) &&
           is_letter(p[1]) &&
           is_digit (p[2]) &&
           is_digit (p[3]) &&
           is_digit (p[4]) &&
           is_letter(p[5]) &&
           is_letter(p[6]);
}

static bool match_uruguai(const char *p)
{
    return is_letter(p[0]) &&
           is_letter(p[1]) &&
           is_letter(p[2]) &&
           is_digit (p[3]) &&
           is_digit (p[4]) &&
           is_digit (p[5]) &&
           is_digit (p[6]);
}

static bool match_paraguai_auto(const char *p)
{
    return is_letter(p[0]) &&
           is_letter(p[1]) &&
           is_letter(p[2]) &&
           is_letter(p[3]) &&
           is_digit (p[4]) &&
           is_digit (p[5]) &&
           is_digit (p[6]);
}

static bool match_paraguai_moto(const char *p)
{
    return is_digit (p[0]) &&
           is_digit (p[1]) &&
           is_digit (p[2]) &&
           is_letter(p[3]) &&
           is_letter(p[4]) &&
           is_letter(p[5]) &&
           is_letter(p[6]);
}

static bool match_bolivia_carga(const char *p)
{
    return is_letter(p[0]) &&
           is_letter(p[1]) &&
           is_digit (p[2]) &&
           is_digit (p[3]) &&
           is_digit (p[4]) &&
           is_digit (p[5]) &&
           is_digit (p[6]);
}

bool plate_is_valid(const char *p)
{
    if (!p)
        return false;

    if (strlen(p) != 7)
        return false;

    return  match_brasil(p)         ||
            match_arg_ven(p)        ||
            match_uruguai(p)        ||
            match_paraguai_auto(p)  ||
            match_paraguai_moto(p)  ||
            match_bolivia_carga(p);
}
