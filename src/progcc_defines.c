#include "progcc_defines.h"

const char* global_string_descriptor[] = {
    // array of pointer to string descriptors
    (char[]){0x09, 0x04},                // 0: is supported language is English (0x0409)
    "HHL",              // 1: Manufacturer
    "ProGCC V3",        // 2: Product
    "000000",           // 3: Serials, should use chip ID

};
