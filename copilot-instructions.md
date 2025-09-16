# C-Coding rules for github copilot

1. A function shall have only one return point
2. Functions which can fail in execution shall return 0 on success, with a non-zero code on failure
3. For integer types of variables and constants the definitions inside stdint.h shall be used
4. Struct definitions shall be implemented using typedef.

## Naming conventions
1. Camel case shall be used for function, variable, struct and typedef naming
2. The type of a variable shall be coded at start of its name: uint8_t **u8**numBit, int16_t **i16**.., struct variables shall start with **st**.
3. Names of typedefs shall start with **t**, in case the typedef is a struct it shall start with **tst**
## Coding style
1.  C code style by Tilen MAJERLE shall be used
