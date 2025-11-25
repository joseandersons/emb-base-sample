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
    // ABC1D23
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
    // AB123CD
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
    // ABC1234
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
    // ABCD123
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
    // 123ABCD
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
    // AB12345
    return is_letter(p[0]) &&
           is_letter(p[1]) &&
           is_digit (p[2]) &&
           is_digit (p[3]) &&
           is_digit (p[4]) &&
           is_digit (p[5]) &&
           is_digit (p[6]);
}

bool plate_is_valid(const char *raw)
{
    if (!raw)
        return false;

    // Normaliza: remove espaços/hífens e deixa só A–Z / 0–9
    char pclean[16];
    size_t j = 0;

    for (size_t i = 0; raw[i] != '\0' && j < sizeof(pclean) - 1; i++) {
        char c = raw[i];

        if (c == ' ' || c == '-' || c == '\t')
            continue;

        if (isalpha((unsigned char)c) || isdigit((unsigned char)c)) {
            pclean[j++] = (char)toupper((unsigned char)c);
        } else {
            // caractere estranho: reprova direto
            return false;
        }
    }

    pclean[j] = '\0';

    // Todos os padrões Mercosul que estamos aceitando têm 7 caracteres
    if (j != 7)
        return false;

    return  match_brasil(pclean)        ||
            match_arg_ven(pclean)       ||
            match_uruguai(pclean)       ||
            match_paraguai_auto(pclean) ||
            match_paraguai_moto(pclean) ||
            match_bolivia_carga(pclean);
}
