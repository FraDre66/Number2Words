#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <ctype.h>
#ifdef _WIN32
#include <windows.h>
#endif
#define MAX_WORD_LEN 128U
#define MAX_RESULT_LEN (1024*1024)
static int number_to_words_small(int n, char *out, size_t out_size);

/** Generates the name of the group using the Cornway / Wechsler system in German (long scale)
 * @param[in] group    Number of 3-digit-group (>=2)
 * @param[out] out     Output buffer for the word
 * @param[in] out_size Size of the output buffer
 * @return 0 on success, 1 on failure
 */
static int cornway_wechsler_german(unsigned int group, char *out, size_t out_size) {
    const char * const first_illions[10]= {"-", "Mi","Bi", "Tri", "Quadri", "Quinti", "Sexti", "Septi", "Okti", "Noni"};
    const char * const name_unit[10] = {"", "un", "do", "tre", "quattuor", "quin", "se", "septe", "okto", "nove"};
    const char * const name_ten[10] = {"", "dezi", "viginti", "triginta", "quadraginta", "quinquaginta", "sexaginta", "septuaginta", "oktoginta", "nonaginta"};
    const char * const name_hundred[10] = {"", "zenti", "duzenti", "trezenti", "quadringenti", "quingenti", "seszenti", "septingenti", "oktingenti", "nongenti"};
    const char * const change_at_ten[10][10] = {
        {"","","","","","","","","",""},
        {"","","","","","","","","",""},
        {"","","","","","","","","",""},
        {"","","s","s","s","s","","","s",""}, // tre(s)
        {"","","","","","","","","",""},
        {"","","","","","","","","",""},
        {"","","s","s","s","s","","","x",""}, // se(s)(x)
        {"","n","m","n","n","n","n","n","m",""}, //septe(n)(m)
        {"","","","","","","","","",""},
        {"","n","m","n","n","n","n","n","m",""} //nove(n)(m)
    };
    const char * const change_at_hundred[10][10] = {
        {"","","","","","","","","",""},
        {"","","","","","","","","",""},
        {"","","","","","","","","",""},
        {"","s","","s","s","s","","","s",""}, //tre(s)
        {"","","","","","","","","",""},
        {"","","","","","","","","",""},
        {"","x","","s","s","s","","","x",""}, //se(s)(x)
        {"","n","n","n","n","n","n","n","m",""}, //septe(n)(m)
        {"","","","","","","","","",""},
        {"","n","n","n","n","n","n","n","m",""} //nove(n)(m)
    };
    char name[MAX_WORD_LEN] = "";
    size_t len = 0;
    int result = 1;
    
    if (group >=2) {
        unsigned int mil_group = group / 2;
        if (mil_group < 10) {
            // first -illions from list:
            len += snprintf(name+len, sizeof(name)-len, "%s", first_illions[mil_group]);
            result = 0;
        } else if (mil_group < 1000) {
            unsigned int unit = mil_group%10;
            unsigned int ten  = (mil_group/10)%10;
            unsigned int hundred  = (mil_group/100)%10;
            const char *unit_change;
            if (ten == 0) {
                unit_change = change_at_hundred[unit][hundred];
            } else {
                unit_change = change_at_ten[unit][ten];
            }
            len += snprintf(name+len, sizeof(name)-len, "%s%s%s%s", name_unit[unit], unit_change, name_ten[ten], name_hundred[hundred]);
            result = 0;
        } else {
            snprintf(out, out_size, "(Number too big!)");
        }
        if (result == 0) {
            name[0] = toupper(name[0]); // first charater to upper case
            // last character has to be replaced by -illion or -illiarde:
            len--;
            len += snprintf(name+len, sizeof(name)-len, "%s", (group%2==0)?"illion":"illiarde");
            snprintf(out, out_size, "%s", name);
        }
    }
    return result;
}

/**
 * @brief Converts an arbitrarily large number given as a string into its German word representation.
 * @param[in] numstr   The number as a string
 * @param[out] out     Output buffer for the word
 * @param[in] out_size Size of the output buffer
 * @return 0 on success, 1 on failure
 */
static int number_string_to_words(const char *numstr, char *out, size_t out_size) {
    int result = 0;
    out[0] = '\0';
    size_t len = strlen(numstr);
    if (len == 0) {
        snprintf(out, out_size, "(not supported)");
        result = 1;
    } else {
        // Check if only digits
        for (size_t i = 0; i < len; i++) {
            if (numstr[i] < '0' || numstr[i] > '9') {
                result = 1;
                break;
            }
        }
        if (result != 0) {
            snprintf(out, out_size, "Invalid input");
        } else {
            unsigned int group_idx = (len-1)/3; // starting with highest 3-digit group (starting with 0)
            unsigned int group_len = (len-1)%3+1; // length of highest 3-digit group (1-3), all other have always 3-digits
            size_t pos = 0;
            size_t tlen = 0;
            do {
                char temp[MAX_WORD_LEN] = "";
                char group_str[MAX_WORD_LEN];
                char cur_group[4];
                unsigned int n;
                for (n=0; n<group_len; n++) cur_group[n] = numstr[pos+n];
                cur_group[n]= 0;
                pos += group_len;
                group_len=3; // all further group will have 3 digit
                n = atoi(cur_group); // value of the current group
                if (n == 0 && group_idx > 0) {
                    continue; // Skip empty groups, except the less significant one
                }
                if (number_to_words_small(n, group_str, sizeof(group_str)) != 0) {
                    snprintf(out, out_size, "(number_to_words_small: not supported)");
                    result = 1;
                    break;
                }
                if (group_idx == 0) {
                    tlen += snprintf(out+tlen, out_size-tlen, "%s", group_str);
                } else if (group_idx == 1) {
                    if (n == 1) {
                        tlen += snprintf(out+tlen, out_size-tlen, "eintausend");
                    } else {
                        tlen += snprintf(out+tlen, out_size-tlen, "%stausend", group_str);
                    }
                } else {
                    result = cornway_wechsler_german(group_idx, temp, sizeof(temp));
                    if (result != 0) {
                        snprintf(out, out_size, "%s", temp);
                        break;
                    }

                    if (n == 1) {
                        tlen += snprintf(out+tlen, out_size-tlen, "eine %s ", temp);
                    } else {
                        char plural[3] = "";
                        //  set plural ending in German uints: illion => illion[en], or illiarde => illiarde[n]
                        // group_idx 2, 4, 6, .. : illion; 3, 5, 7, .. illiarde
                        if (group_idx%2 == 0) {
                            plural[0] = 'e'; plural[1] = 'n'; plural[2] = '\0';
                        } else {
                            plural[0] = 'n'; plural[1] = '\0';
                        } 
                        tlen += snprintf(out+tlen, out_size-tlen, "%s %s%s ", group_str, temp, plural);
                    }
                }
            } while (group_idx-- > 0);
        }
    }
    return result;
}

/**
 * @brief Converts a number < 1000 into its German word representation.
 * @param[in] n        The number
 * @param[out] out     Output buffer for the word
 * @param[in] out_size Size of the output buffer
 * @return 0 on success, 1 on failure
 */
static int number_to_words_small(int n, char *out, size_t out_size) {
    const char * const units[] = {"null", "eins", "zwei", "drei", "vier", "fünf", "sechs", "sieben", "acht", "neun"};
    const char * const teens[] = {"zehn", "elf", "zwölf", "dreizehn", "vierzehn", "fünfzehn", "sechzehn", "siebzehn", "achtzehn", "neunzehn"};
    const char * const tens[] = {"", "zehn", "zwanzig", "dreißig", "vierzig", "fünfzig", "sechzig", "siebzig", "achtzig", "neunzig"};
    int result = 1;
    out[0] = '\0';
    if (n < 0 || n >= 1000) {
        snprintf(out, out_size, "(not supported)");
        result = 1;
    } else if (n == 0) {
        snprintf(out, out_size, "%s", units[0]);
        result = 0;
    } else {
        char temp[MAX_WORD_LEN] = "";
        size_t len = 0U;
        int hundred = n / 100;
        int tenunit = n % 100;
        if (hundred > 0) {
            if (hundred == 1) {
                len += snprintf(temp+len, sizeof(temp)-len, "einhundert");
            } else {
                len += snprintf(temp+len, sizeof(temp)-len, "%shundert", units[hundred]);
            }
        }
        if (tenunit > 0) {
            if (tenunit < 10) {
                if (tenunit == 1 && len > 0) {
                    len += snprintf(temp+len, sizeof(temp)-len, "eins");
                } else {
                    len += snprintf(temp+len, sizeof(temp)-len, "%s", units[tenunit]);
                }
            } else if (tenunit < 20) {
                len += snprintf(temp+len, sizeof(temp)-len, "%s", teens[tenunit-10]);
            } else {
                int ten = tenunit / 10;
                int unit = tenunit % 10;
                if (unit == 0) {
                    len += snprintf(temp+len, sizeof(temp)-len, "%s", tens[ten]);
                } else {
                    if (unit == 1) {
                        len += snprintf(temp+len, sizeof(temp)-len, "einund%s", tens[ten]);
                    } else {
                        len += snprintf(temp+len, sizeof(temp)-len, "%sund%s", units[unit], tens[ten]);
                    }
                }
            }
        }
        snprintf(out, out_size, "%s", temp);
        result = 0;
    }
    return result;
}

/**
 * @brief Main entry point. Converts all command line numbers to German words.
 * @param[in] argc  Number of arguments
 * @param[in] argv  Argument vector
 * @return 0 on success, 1 on failure
 */
int main(int argc, char *argv[]) {
    int result = 0;
#ifdef _WIN32
    SetConsoleOutputCP(65001); // UTF-8 for Windows console
#endif
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number> [number ...]\n", argv[0]);
        result = 1;
    } else {
        for (int i = 1; i < argc; i++) {
            char word[MAX_RESULT_LEN];
            int conv_result = number_string_to_words(argv[i], word, sizeof(word));
            if (conv_result == 0) {
                printf("%s: %s\n", argv[i], word);
            } else {
                printf("%s: Error: %s\n", argv[i], word);
                result = 1;
            }
        }
    }
    return result;
}



