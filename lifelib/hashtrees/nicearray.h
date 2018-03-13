/*
* Represents a hashable, comparable array type.
*/

#pragma once
#include <string>
#include <cstring>
#include <stdint.h>
#include <initializer_list>
#include "base85.h"

namespace apg {

    template <typename T, int N>
    struct nicearray {

        T x[N];

        const static int k64 = (sizeof(T) * N) / 8;
        const static int k32 = (sizeof(T) * N) / 4;

        uint64_t hash() {

            uint64_t hcopy[k64];
            std::memcpy(hcopy, x, 8 * k64);

            uint64_t h = hcopy[0];
            for (int i = 1; i < k64; i++) {
                h -= (h << 7);
                h += hcopy[i];
            }

            return h;
        }

        bool iszero() {
            int i = N;
            while (i --> 0) // i goes to zero
            {
                if (x[i] != 0) { return false; }
            }
            return true;
        }

        void fromBase85(std::string str) {

            uint32_t hcopy[k32];
            std::memset(hcopy, 0, 4 * k32);
            base85decode(hcopy, str, k32);
            std::memcpy(x, hcopy, 4 * k32);

        }

        std::string toBase85() {

            std::string str = "";
            uint32_t hcopy[k32];
            std::memcpy(hcopy, x, 4 * k32);

            for (int i = 0; i < k32; i++) {
                str += base85encode(hcopy[i]);
            }

            str = str.substr(0, 1 + str.find_last_not_of('.'));
            if (str.length() == 0) { str = "."; }

            return str;

        }

        template<typename ...U>
        nicearray(U... ts) : x{ts...} { }

    };

    template <typename T, int N>
    bool operator==(const nicearray<T, N> &l, const nicearray<T, N> &r) {

        int i = N;
        while (i --> 0) // i goes to zero
        {
            if (l.x[i] != r.x[i]) { return false; }
        }
        return true;

    }

}
