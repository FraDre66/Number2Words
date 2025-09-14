#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#ifdef _WIN32
#include <windows.h>
#endif
#define MAX_WORD_LEN 128U
static const char * const units[] = {
    "null", "eins", "zwei", "drei", "vier", "fünf", "sechs", "sieben", "acht", "neun"
};
static const char * const teens[] = {
    "zehn", "elf", "zwölf", "dreizehn", "vierzehn", "fünfzehn", "sechzehn", "siebzehn", "achtzehn", "neunzehn"
};
static const char * const tens[] = {
    "", "zehn", "zwanzig", "dreißig", "vierzig", "fünfzig", "sechzig", "siebzig", "achtzig", "neunzig"
};
static const char * const big_units[] = {"einer",
  "tausend", "Million", "Milliarde", "Billion", "Billiarde", "Trillion", "Trilliarde", "Quadrillion", "Quadrilliarde",
  "Quintillion", "Quintilliarde", "Sextillion", "Sextilliarde", "Septillion", "Septilliarde", "Oktillion", "Oktilliarde",
  "Nonillion", "Nonilliarde", "Dezillion", "Dezilliarde", "Undezillion", "Undezilliarde", "Dodezillion", "Dodezilliarde", 
  "Tredezillion", "Tredezilliarde", "Quattuordezillion", "Quattuordezilliarde", "Quindezillion", "Quindezilliarde", 
  "Sedezillion", "Sedezilliarde", "Septendezillion", "Septendezilliarde", "Oktodezillion", "Oktodezilliarde", "Novemdezillion",
  "Novemdezilliarde", "Vigintillion", "Vigintilliarde", "Unvigintillion", "Unvigintilliarde", "Dovigintillion", "Dovigintilliarde",
  "Tresvigintillion", "Tresvigintilliarde", "Quattuorvigintillion", "Quattuorvigintilliarde", "Quinvigintillion", "Quinvigintilliarde",
  "Sevigintillion", "Sevigintilliarde", "Septenvigintillion", "Septenvigintilliarde", "Dodetrigintillion", "Dodetrigintillarde",
  "Undetrigintillion", "Undetrigintilliarde", "Trigintillion", "Trigintilliarde", "Untrigintillion", "Untrigintilliarde",
  "Dotrigintillion", "Dotrigintilliarde", "Tretrigintillion", "Tretrigintilliarde", "Quattuortrigintillion", "Quattuortrigintilliarde",
  "Quintrigintillion", "Quintrigintilliarde", "Setrigintillion", "Setrigintilliarde", "Septentrigintillion", "Septentrigintilliarde",
  "Oktotrigintillion", "Oktotrigintilliarde", "Novemtrigintillion", "Novemtrigintilliarde", "Quadragintillion", "Quadragintilliarde",
  "Unquadragintillion", "Unquadragintilliarde", "Doquadragintillion", "Doquadragintilliarde", "Trequadragintillion",
  "Trequadragintilliarde", "Quattuorquadragintillion", "Quattuorquadragintilliarde", "Quinquadragintillion", "Quinquadragintilliarde",
  "Sequadragintillion", "Sequadragintilliarde", "Septenquadragintillion", "Septenquadragintilliarde", "Oktoquadragintillion",
  "Oktoquadragintilliarde", "Novemquadragintillion", "Novemquadragintilliarde", "Quinquagintillion", "Quinquagintilliarde",
  "Unquinquagintillion", "Unquinquagintilliarde", "Doquinquagintillion", "Doquinquagintilliarde", "Tresquinquagintillion",
  "Tresquinquagintilliarde", "Quattuorquinquagintillion", "Quattuorquinquagintilliarde", "Quinquinquagintillion", "Quinquinquagintilliarde",
  "Sesquinquagintillion", "Sesquinquagintilliarde", "Septenquinquagintillion", "Septenquinquagintilliarde", "Oktoquinquagintillion", "Oktoquinquagintilliarde",
  "Novenquinquagintillion", "Novenquinquagintilliarde", "Sexagintillon", "Sexagintilliarde",
  "Unsexagintillon", "Unsexagintilliarde","Dosexagintillon", "Dosexagintilliarde",
};
#define MAX_GROUPS (sizeof(big_units) / sizeof(big_units[0]))
#define MAX_RESULT_LEN (MAX_GROUPS * MAX_WORD_LEN)
static int number_to_words_small(int n, char *out, size_t out_size);

/**
 * @brief Converts an arbitrarily large number given as a string into its German word representation.
 * @param[in] numstr   The number as a string
 * @param[out] out     Output buffer for the word
 * @param[in] out_size Size of the output buffer
 * @return 0 on success, 1 on failure
 */
static int number_string_to_words(const char *numstr, char *out, size_t out_size) {
    int result = 1;
    out[0] = '\0';
    size_t len = strlen(numstr);
    if (len == 0) {
        snprintf(out, out_size, "(not supported)");
        result = 1;
    } else {
        // Check if only digits
        int valid = 1;
        for (size_t i = 0; i < len; i++) {
            if (numstr[i] < '0' || numstr[i] > '9') {
                valid = 0;
                break;
            }
        }
        if (!valid) {
            snprintf(out, out_size, "Invalid input");
            result = 1;
        } else {
            // Split into groups of three digits from the end
            char groups[MAX_GROUPS][4] = {{0}};
            unsigned int group_count = 0;
            int pos = (int)len;
            while (pos > 0 && group_count < MAX_GROUPS) {
                int start = (pos >= 3) ? pos-3 : 0;
                int glen = pos-start;
                strncpy(groups[group_count], numstr+start, glen);
                groups[group_count][glen] = '\0';
                pos -= 3;
                group_count++;
            }
            if (pos > 0) {
                snprintf(out, out_size, "(number too large)! Number must less than 10^%d", 3*MAX_GROUPS);
                return 1;
            }
            char temp[MAX_RESULT_LEN] = "";
            size_t tlen = 0U;
            int error = 0;
            for (int i = group_count-1; i >= 0; i--) {
                int val = atoi(groups[i]);
                char part_word[MAX_WORD_LEN] = "";
                if (val == 0 && group_count > 1) {
                    continue; // Skip empty parts
                }
                if (number_to_words_small(val, part_word, sizeof(part_word)) != 0) {
                    snprintf(out, out_size, "(number_to_words_small: not supported)");
                    error = 1;
                    break;
                }
                if (i == 0) {
                    tlen += snprintf(temp+tlen, sizeof(temp)-tlen, "%s", part_word);
                } else if (i == 1) {
                    if (strcmp(part_word, "eins") == 0) {
                        tlen += snprintf(temp+tlen, sizeof(temp)-tlen, "eintausend");
                    } else {
                        tlen += snprintf(temp+tlen, sizeof(temp)-tlen, "%stausend", part_word);
                    }
                } else {
                    if (val == 1) {
                        tlen += snprintf(temp+tlen, sizeof(temp)-tlen, "eine %s ", big_units[i]);
                    } else if (val > 0) {
                        char plural[3] = "";
                        //  set plural ending in German uints: illio[n] => illion[en], or illiard[e] => illarde[n]
                        if (big_units[i][strlen(big_units[i])-1] == 'e') {
                            plural[0] = 'n'; plural[1] = '\0';
                        } else if (big_units[i][strlen(big_units[i])-1] == 'n') {
                            plural[0] = 'e'; plural[1] = 'n'; plural[2] = '\0';
                        } else {
                            snprintf(out, out_size, "(big_units[%d]: '%s' internal error, misspelled unit)",i , big_units[i]);
                            error = 1;
                            break;
                        }
                        tlen += snprintf(temp+tlen, sizeof(temp)-tlen, "%s %s%s ", part_word, big_units[i], plural);
                    }
                }
            }
            if (!error) {
                snprintf(out, out_size, "%s", temp);
                result = 0;
            } else {
                result = 1;
            }
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

