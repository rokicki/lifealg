/*
* Modular exponentiation and primality testing for 64-bit integers.
* Adam P. Goucher, 2016
*/

#pragma once

#include <stdint.h>
#include <iostream>
#include <vector>
#include <cstdio>
#include <cmath>
#include <string>
#include <map>

namespace apg {

    // Rename GCC's builtin __uint128_t for consistency with stdint:
    typedef __uint128_t uint128_t;

    uint32_t modexp_u32(uint32_t x, uint32_t y, uint32_t modulus) {

        uint64_t z = 1;
        uint64_t t = x;
        uint32_t w = y;
        while (w) {
            if (w & 1) {
                z = (z * t) % modulus;
            }
            t = (t * t) % modulus;
            w = w >> 1;
        }

        return (uint32_t) z;
    }

    uint64_t modexp_u64(uint64_t x, uint64_t y, uint64_t modulus) {

        uint128_t z = 1;
        uint128_t t = x;
        uint64_t w = y;
        while (w) {
            if (w & 1) {
                z = (z * t) % modulus;
            }
            t = (t * t) % modulus;
            w = w >> 1;
        }

        return (uint64_t) z;
    }

    bool mrabin32(uint32_t a, uint32_t p) {

        // Miller-Rabin gives no information if p divides a:
        if ((a % p) == 0) { return true; }

        int r = __builtin_ctzll(p - 1);
        uint32_t d = p >> r;
        uint64_t e = modexp_u32(a, d, p);
        if ((e == 1) || (e + 1 == p)) { return true; }
        for (int i = 1; i < r; i++) {
            e = (e * e) % p;
            if (e == 1) { return false; }
            if (e + 1 == p) { return true; }
        }

        // Fails Fermat primality test, never mind Miller-Rabin:
        return false;
    }

    bool mrabin64(uint64_t a, uint64_t p) {

        // Miller-Rabin gives no information if p divides a:
        if ((a % p) == 0) { return true; }

        int r = __builtin_ctzll(p - 1);
        uint64_t d = p >> r;
        uint128_t e = modexp_u64(a, d, p);
        if ((e == 1) || (e + 1 == p)) { return true; }
        for (int i = 1; i < r; i++) {
            e = (e * e) % p;
            if (e == 1) { return false; }
            if (e + 1 == p) { return true; }
        }

        // Fails Fermat primality test, never mind Miller-Rabin:
        return false;
    }

    // We want these arrays to be permanent, immutable, and aligned on
    // 64-byte cache boundaries:

    const static uint8_t __res30[] __attribute__((aligned(64))) = {8,
        0,8,8,8,8,8,1,8,8,8,2,8,3,8,8,8,4,8,5,8,8,8,6,8,8,8,8,8,7};

    const static uint8_t __preatkin[] __attribute__((aligned(64))) = {254,
        223, 239, 126, 182, 219, 61, 249, 213, 79, 30, 243, 234, 166, 237,
        158, 230, 12, 211, 211, 59, 221, 89, 165, 106, 103, 146, 189, 120,
        30, 166, 86, 86, 227, 173, 45, 222, 42, 76, 85, 217, 163, 240, 159,
        3, 84, 161, 248, 46, 253, 68, 233, 102, 246, 19, 58, 184, 76, 43,
        58, 69, 17, 191, 84};

    bool isprime64(uint64_t p) {
        /*
        * Tests the primality of any unsigned 64-bit integer:
        */

        if (p <= 63) {
            // Just read it out of a lookup table (we need this to special-
            // case the numbers 2, 3 and 5 which would otherwise fail the
            // modulo-30 trial division test):
            return (1 & (2891462833508853932ll >> p));
        } else {
            // GCC will do these in one machine instruction:
            uint64_t q = (p / 30);
            uint64_t r = (p % 30);

            // We use __res30[r] to index into the bits of __preatkin[q]:
            uint8_t s = __res30[r];

            if (s == 8) {
                // Number is divisible by 2, 3 or 5:
                return false;
            } else if (p < 1920) {
                // Larger (64-byte) lookup table:
                return (__preatkin[q] & (1 << s));
            } else if ((p % 7 == 0) || (p % 11 == 0) || (p % 13 == 0)) {
                // Number is divisible by 7, 11 or 13:
                return false;
            } else if (p < 49140) {
                // Best 32-bit single-base Miller-Rabin primality test:
                return mrabin32(921211727, p);
            } else if (p < 316349280) {
                // Not necessarily the best two-base test, but quite good:
                return mrabin32(11000544, p) && mrabin32(31481107, p);
            } else if (p < 0xffffffffull) {
                // Full 32-bit Miller-Rabin exhaustion:
                return mrabin32(2, p) && mrabin32(7, p) && mrabin32(61, p);
            } else if (p < 350269456336ull) {
                // Three-base 64-bit Miller-Rabin exhaustion:
                return (mrabin64( 4230279247111683200ull, p) &&
                        mrabin64(14694767155120705706ull, p) &&
                        mrabin64(16641139526367750375ull, p));
            } else {
                // Full 64-bit Miller-Rabin exhaustion (probably improvable):
                return (mrabin64(2, p) && mrabin64(325, p) &&
                        mrabin64(9375, p) && mrabin64(28178, p) &&
                        mrabin64(450775, p) && mrabin64(9780504, p) &&
                        mrabin64(1795265022, p));
            }
        }
    }

    std::vector<int64_t> moebius_mu(uint64_t limit) {
        /*
        * Compute an initial segment of the Moebius mu function.
        */
        std::vector<int64_t> mu(limit, 1);
        for (uint64_t p = 2; p < limit; p++) {
            if (isprime64(p)) {
                for (uint64_t q = 0; q < limit; q += p) {
                    mu[q] = 0 - mu[q];
                }
                if (p < 0x100000000ull) {
                    // Check necessary because uint64_t overflow.
                    uint64_t psq = p * p;
                    for (uint64_t q = 0; q < limit; q += psq) {
                        mu[q] = 0;
                    }
                }
            }
        }
        return mu;
    }

    uint64_t nextprime(uint64_t n) {

        uint64_t p = n + 1 + (n % 2);
        while (!isprime64(p)) { p += 2; }
        return p;

    }

    uint64_t euclid_gcd(uint64_t a, uint64_t b) {
        uint64_t c = (a < b) ? b : a;
        uint64_t d = a ^ b ^ c;

        while (d) {
            c = c % d;
            if (c == 0) { return d; }
            d = d % c;
        }
        return c;
    }

    uint64_t euclid_lcm(uint64_t a, uint64_t b) {
        return (a / euclid_gcd(a, b)) * b;
    }

    uint64_t pollard_rho(uint64_t N, uint64_t initial, int bailout, int perloop) {

        uint128_t x = initial % N;
        uint128_t y = initial % N;

        uint128_t M = 1;

        for (int i = 0; i < bailout; i++) {

            // GCC, please unroll this loop:
            for (int j = 0; j < perloop; j++) {
                x = (x * x + 1) % N;
                y = (y * y + 1) % N;
                y = (y * y + 1) % N;
                M = (M * ((x-y) + N)) % N;
            }

            uint64_t g = euclid_gcd(M, N);
            if (g != 1) { return (g % N); }
        }

        // Failed to find a factor:
        return 0;

    }

    // Also align this on a 64-byte cache boundary:
    const static uint16_t __smallprimes[] __attribute__((aligned(64))) = {2, 3,
        5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
        73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
        151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227,
        229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307,
        311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389,
        397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467,
        479, 487, 491, 499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571,
        577, 587, 593, 599, 601, 607, 613, 617, 619, 631, 641, 643, 647, 653,
        659, 661, 673, 677, 683, 691, 701, 709, 719};

    uint64_t find_prime_factor(uint64_t N) {
        // Finds a prime factor, not necessarily the smallest.

        if (isprime64(N)) {
            return N;
        }

        for (int i = 0; i < 128; i++) {
            uint64_t j = __smallprimes[i];
            if (N % j == 0) {
                return j;
            }
        }

        // If N is not divisible by a prime factor < 566, and has only one
        // distinct prime factor, then it must be a power of 2, 3, or 5:
        uint64_t x = std::sqrt(N) + 0.5;
        if ((x * x) == N) { return find_prime_factor(x); }
        x = std::pow(N, 1.0 / 3) + 0.5;
        if ((x * x * x) == N) { return find_prime_factor(x); }
        x = std::pow(N, 1.0 / 5) + 0.5;
        if ((x * x * x * x * x) == N) { return find_prime_factor(x); }

        uint64_t candidate = 0;
        uint64_t initial = 1;
        int bailout = 4096;

        while (candidate == 0) {
            candidate = pollard_rho(N, initial, bailout, 16);
            if (candidate == 0) { candidate = pollard_rho(N, initial, bailout, 1); }
            initial += 2;
            bailout += 4096;
            // std::cerr << "bailout = " << bailout << std::endl;
        }

        // No guarantee that the output of Pollard's rho is prime; iterate:
        candidate = find_prime_factor(candidate);

        return candidate;

    }

    std::map<uint64_t, uint64_t> factorise64(uint64_t N) {
        // Finds the full factorisation N = p_1^a_1 ... p_k^a_k and outputs
        // it as a map {p_1: a_1, ..., p_k: a_k}.

        uint64_t rem = N;
        std::map<uint64_t, uint64_t> mmap;

        if ((rem & 1) == 0) {
            uint64_t v2 = __builtin_ctzll(rem);
            rem = rem >> v2;
            mmap.emplace(2, v2);
        }

        while (rem > 1) {
            uint64_t p = find_prime_factor(rem);
            do {
                rem = rem / p;
                mmap[p] += 1;
                // In case the prime factor occurs with multiplicity:
            } while ((rem % p) == 0);
        }

        return mmap;
    }

    uint64_t sumof2squares(uint64_t x) {
        /*
        * Determine the number of ways to express a positive integer as the
        * sum of two squares.
        */

        if (x == 0) { return 1; }

        uint64_t y = (x >> __builtin_ctzll(x));
        auto facs = factorise64(y);
        uint64_t z = 4;

        for (auto it = facs.begin(); it != facs.end(); ++it) {
            if ((it->first & 3) == 3) {
                if (it->second & 1) { return 0; }
            } else {
                z *= (it->second + 1);
            }
        }

        return z;
    }

    std::vector<uint64_t> divisors64(uint64_t N) {
        // Returns a list of all divisors of N, including 1 and N.
        // If two divisors multiply to give N, then their positions
        // in the list sum to length - 1. [N.B. the divisors do not
        // appear in numerical order, instead mimicking a strided
        // multidimensional array giving the divisibility lattice.]

        std::map<uint64_t, uint64_t> pfac = apg::factorise64(N);
        std::vector<uint64_t> divs; divs.push_back(1);
        for (auto it = pfac.begin(); it != pfac.end(); ++it) {
            uint64_t p = it->first;
            uint64_t k = it->second * divs.size();
            for (uint64_t i = 0; i < k; i++) { divs.push_back(p * divs[i]); }
        }
        return divs;
    }
}

/*
int main() {

    std::string str(".g...");
    std::cout << str.find_last_not_of('.') << std::endl;

    int j = 0;

    std::printf("Array pointers: %p %p\n", apg::__res30, apg::__preatkin);
    std::cout << "  __res30 size: " << sizeof(apg::__res30) << std::endl;
    std::cout << "  __preatkin size: " << sizeof(apg::__preatkin);
    std::cout << std::endl << std::endl << "Calculating..." << std::endl;

    for (uint64_t i = 0x4000000000000000ll; i < 0x40000000000fffff; i++) {
        j += apg::isprime64(i);
    }
    std::cout << "Total primes between 2^62 and 2^62 + 2^20: " << j << std::endl;

    j = 0;
    for (uint64_t i = 0x40000000ll; i < 0x400fffff; i++) {
        j += apg::isprime64(i);
    }
    std::cout << "Total primes between 2^30 and 2^30 + 2^20: " << j << std::endl;

    std::cout << apg::find_prime_factor(10000000089000000133ull) << std::endl;
    return 0;

}
*/
