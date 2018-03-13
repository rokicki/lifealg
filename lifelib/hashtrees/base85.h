/*
* Base85 -- A method to allow 32-bit integers to be conveniently
* encoded as strings. We use a period '.' to represent the value 0,
* or a five-character string to represent any other value. This is
* similar to base64, but more efficient (80% instead of 75%).
*
* For representing an array of 32-bit integers, we convert each
* element separately, concatenate the results, and optionally discard
* any trailing periods.
*
* For representing an arbitrary binary stream, we memcpy it into a
* sufficiently large array of 32-bit integers and encode it as above.
*
* Adam P. Goucher, August 2016
*/

#pragma once

#include <string>
#include <stdint.h>

namespace apg {

    // We take the 85 printable characters which are not whitespace,
    // brackets/braces/parentheses, backslashes, or quotes. 85 is the
    // smallest n such that n^5 > 2^32.
    static const char __base85[] __attribute__((aligned(64))) = {'0', '1',
        '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
        'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
        'W', 'X', 'Y', 'Z', '!', '^', '#', '$', '%', '&', '|', '*', '+', ',',
        '-', '~', '/', ':', ';', '<', '=', '>', '?', '_', '`', '@', '.'};

    std::string base85encode(uint32_t x) {
        if (x == 0) {
            return ".";
        } else {
            char arr[ ] = "00000";
            int i = 5;
            while (i --> 0) {
                arr[i] = __base85[x % 85];
                x = x / 85;
            }
            std::string str(arr);
            return str;
        }
    }

    int base85decode(uint32_t* arr, std::string str, uint32_t limit) {

        int decoder[256];

        unsigned int j = 0;
        unsigned int i = 0;

        for (int k = 0; k < 85; k++) {
            decoder[(int) __base85[k]] = k;
        }

        while ((i < str.length()) && (j < limit)) {
            arr[j] = 0;
            if (str[i] == '.') {
                i += 1;
            } else {
                for (int k = 0; k < 5; k++) {
                    arr[j] *= 85;
                    arr[j] += ((i == str.length()) ? 84 : decoder[(int) str[i++]]);
                }
            }
            j += 1;
        }

        // Returns the length of the array:
        return j;
    }

    int base85decode(uint32_t* arr, std::string str) {
        return base85decode(arr, str, 0xffffffffu);
    }

}


/*
int main() {

    for (uint32_t i = 0; i < 0x80; i++) {
        std::cout << apg::base85encode(i);
        if (i % 16 == 15) { std::cout << std::endl; }
    }
    std::cout << std::endl;
    for (uint32_t i = 0xffffff80; i != 0; i++) {
        std::cout << apg::base85encode(i);
        if (i % 16 == 15) { std::cout << std::endl; }
    }
    uint32_t arr[16];
    std::cout << apg::base85decode(arr, "00001000020000300004....`nSb``nSb@`nSb.`nSc0") << std::endl;
    for (int i = 0; i < 12; i++) {
        std::cout << arr[i] << std::endl;
    }

    return 0;

}
*/
