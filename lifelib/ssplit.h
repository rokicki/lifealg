#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

namespace apg {

    template<typename Out>
    void string_split(const std::string &s, char delim, Out result) {
        std::stringstream ss;
        ss.str(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            *(result++) = item;
        }
    }

    std::vector<std::string> string_split(const std::string &s, char delim) {
        std::vector<std::string> elems;
        string_split(s, delim, std::back_inserter(elems));
        return elems;
    }

    void onlyints(std::vector<int64_t> &v, std::istream &in) {
        while (true) {
            int ch = in.get();
            if (ch == EOF) { break; }
            if ((ch >= '0') && (ch <= '9')) {
                int64_t val = ch - '0';
                while (true) {
                    ch = in.get();
                    if ((ch < '0') || (ch > '9')) { break; }
                    val *= 10;
                    val += ch - '0';
                }
                v.push_back(val);
            }
        }
    }
}
