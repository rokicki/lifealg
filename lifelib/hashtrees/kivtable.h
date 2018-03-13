/*
* (key, index, value) hashtables that can be addressed by either the
* key (like a regular associative array) or the (typically 32-bit)
* index integer.
*/

#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include <cstring>
#include "numtheory.h"
#include <iostream>

#ifdef __CYGWIN__
#include <cstdlib>
#include <cerrno>
#define posix_memalign(p, a, s) (((*(p)) = std::malloc((s))), *(p) ? 0 : errno)
#endif

namespace apg {

    const static uint32_t klowbits = 12;

    template <typename K, typename I, typename V>
    struct kiventry {

        K key;
        I next;
        I gcflags; // For gc and ensuring nice standard layout.
        V value;

    };

    template <typename K, typename I, typename V>
    class kivtable {

        std::vector<kiventry<K, I, V>*> arraylist;
        uint64_t hashprime;
        uint64_t totalnodes;
        I* hashtable;
        I freenodes;

        public:
        I gccounter;

        // Convert index to pointer:
        kiventry<K, I, V>* ind2ptr(I index) {
            return (arraylist[index >> klowbits] + (index & ((1 << klowbits) - 1)));
        }

        uint64_t max_size() {
            return arraylist.max_size();
        }

        void resize_hash(uint64_t newprime) {

            if (newprime == hashprime) {
                return;
            }

            // Create a new hashtable:
            I* newtable;
            if (posix_memalign((void**) &newtable, 64, sizeof(I) * newprime)) {
                std::cerr << "Memory error!!!" << std::endl;
                return;
            }
            std::memset(newtable, 0, sizeof(I) * newprime);

            // Based on code by Tom Rokicki:
            for (uint64_t i = 0; i < hashprime; i++) {
                I p = hashtable[i];
                while (p) {
                    kiventry<K, I, V>* pptr = ind2ptr(p);
                    I np = pptr->next;
                    uint64_t h = pptr->key.hash() % newprime;
                    pptr->next = newtable[h] ;
                    newtable[h] = p;
                    p = np; // contrary to the beliefs of most complexity theorists.
                }
            }

            free(hashtable);
            hashtable = newtable;
            hashprime = newprime;

        }

        private:

        void resize_if_necessary() {
            if (totalnodes > hashprime) {
                uint64_t newprime = apg::nextprime(hashprime * 3);
                // std::cerr << "Resizing hashtable to size " << newprime << " ... ";
                resize_hash(newprime);
                // std::cerr << "done!" << std::endl;
            }
        }

        // Get the next free node:
        I getfreenode() {
            if (freenodes == 0) {

                freenodes = arraylist.size() << klowbits;
                kiventry<K, I, V>* nextarray;
                if (posix_memalign((void**) &nextarray, 64, sizeof(kiventry<K, I, V>) << klowbits)) {
                    std::cerr << "Memory error!!!" << std::endl;
                    return 0;
                }
                std::memset(nextarray, 0, sizeof(kiventry<K, I, V>) << klowbits);
                arraylist.push_back(nextarray);

                for (int i = 0; i < ((1 << klowbits) - 1); i++) {
                    nextarray[i].next = freenodes + i + 1;
                    nextarray[i].gcflags = 0;
                }

            }
            return freenodes;
        }

        // New node with empty value:
        I newnode(K key, I next) {
            I r = getfreenode();
            kiventry<K, I, V>* fptr = ind2ptr(r);
            freenodes = fptr->next;
            fptr->key = key;
            fptr->next = next;
            totalnodes += 1;
            return r;
        }

        // New node with specified value:
        I newnode(K key, I next, V value) {
            I r = getfreenode();
            kiventry<K, I, V>* fptr = ind2ptr(r);
            freenodes = fptr->next;
            fptr->key = key;
            fptr->next = next;
            fptr->value = value;
            totalnodes += 1;
            return r;
        }

        public:

        uint64_t size() {
            return totalnodes;
        }

        uint64_t total_bytes() {
            uint64_t nodemem = sizeof(kiventry<K, I, V>) * totalnodes;
            uint64_t hashmem = sizeof(I) * hashprime;
            return (nodemem + hashmem);
        }

        bool erasenode(K key) {

            if (key.iszero()) { return 0; }
            uint64_t h = key.hash() % hashprime;

            I p = hashtable[h];
            kiventry<K, I, V>* predptr = 0;
            while (p) {
                kiventry<K, I, V>* pptr = ind2ptr(p);
                if (pptr->key == key) {

                    // Remove from hashtable:
                    if (predptr) {
                        predptr->next = pptr->next;
                    } else {
                        hashtable[h] = pptr->next;
                    }

                    // Reset memory:
                    std::memset(pptr, 0, sizeof(kiventry<K, I, V>));

                    // Prepend to list of free nodes:
                    pptr->next = freenodes;
                    freenodes = p;

                    // We've reduced the total number of nodes:
                    totalnodes -= 1;
                    return true;
                }
            }

            // Node did not exist ab initio:
            return false;
        }

        // Get node index from key:
        I getnode(K key, bool makenew) {

            // If the key is zero, return zero; otherwise make a right hash:
            if (key.iszero()) { return 0; }
            uint64_t h = key.hash() % hashprime;

            I p = hashtable[h];
            kiventry<K, I, V>* predptr = 0;
            while (p) {
                kiventry<K, I, V>* pptr = ind2ptr(p);
                if (pptr->key == key) {
                    if (predptr) {
                        // Move this node to the front:
                        predptr->next = pptr->next;
                        pptr->next = hashtable[h];
                        hashtable[h] = p;
                    }
                    return p;
                }
                predptr = pptr;
                p = pptr->next;
            }
            if (makenew) {
                p = newnode(key, hashtable[h]);
                hashtable[h] = p;
                resize_if_necessary();
                return p;
            } else {
                return -1;
            }
        }

        // Create a (key, value) pair and return index:
        I setnode(K key, V value) {

            // If the key is zero, return zero; otherwise make a right hash:
            if (key.iszero()) { return 0; }
            uint64_t h = key.hash() % hashprime;

            I p = hashtable[h];
            kiventry<K, I, V>* predptr = 0;
            while (p) {
                kiventry<K, I, V>* pptr = ind2ptr(p);
                if (pptr->key == key) {
                    // Set the value:
                    pptr->value = value;
                    if (predptr) {
                        // Move this node to the front:
                        predptr->next = pptr->next;
                        pptr->next = hashtable[h];
                        hashtable[h] = p;
                    }
                    return p;
                }
                predptr = pptr;
                p = pptr->next;
            }
            p = newnode(key, hashtable[h], value);
            hashtable[h] = p;
            resize_if_necessary();
            return p;
        }

        void gc_traverse(bool destructive) {
            /*
            * Run gc_traverse(false) to zero all gcflags;
            * Run gc_traverse(true) to delete all items with zero gcflags.
            */

            for (uint64_t i = 0; i < hashprime; i++) {
                I p = hashtable[i];
                kiventry<K, I, V>* predptr = 0;
                while (p) {
                    kiventry<K, I, V>* pptr = ind2ptr(p);
                    I np = pptr->next;
                    if (destructive && pptr->gcflags == 0) {
                        // Remove from hashtable:
                        if (predptr) {
                            predptr->next = np;
                        } else {
                            hashtable[i] = np;
                        }

                        // Reset memory:
                        std::memset(pptr, 0, sizeof(kiventry<K, I, V>));

                        // Prepend to list of free nodes:
                        pptr->next = freenodes;
                        freenodes = p;

                        // We've reduced the total number of nodes:
                        totalnodes -= 1;
                    } else {
                        // Node still exists; zero the flags:
                        pptr->gcflags = 0;
                        predptr = pptr;
                    }
                    // Contrary to the belief of most complexity theorists:
                    p = np;
                }
            }

            gccounter = 0;
        }


        void init(uint64_t hashprime) {

            // std::cout << "Initialising kivtable with p = " << hashprime << std::endl;

            this->hashprime = hashprime;
            if (posix_memalign((void**) &hashtable, 64, sizeof(I) * hashprime)) {
                std::cerr << "Memory error!!!" << std::endl;
                exit(1);
            }
            std::memset(hashtable, 0, sizeof(I) * hashprime);

            // Allocate initial block of 4096 entries:
            kiventry<K, I, V>* nextarray;
            if (posix_memalign((void**) &nextarray, 64, sizeof(kiventry<K, I, V>) << klowbits)) {
                std::cerr << "Memory error!!!" << std::endl;
                exit(1);
            }
            std::memset(nextarray, 0, sizeof(kiventry<K, I, V>) << klowbits);
            arraylist.push_back(nextarray);

            for (int i = 1; i < ((1 << klowbits) - 1); i++) {
                nextarray[i].next = i + 1;
            }

            gccounter = 0;
            totalnodes = 0;
            freenodes = 1;
        }

        kivtable(uint64_t hashprime) { init(hashprime); }

        kivtable() { init(3511); /* Wieferich prime */ }

        ~kivtable() {

            // std::cout << "Calling destructor" << std::endl;

            // Destroy all the memory we malloc'd:
            while (!arraylist.empty()) {
                free(arraylist.back());
                arraylist.pop_back();
            }

            // Free the hashtable itself:
            free(hashtable);

        }

    };

    /*
    template <typename K, typename V>
    using kivtable32 = kivtable<K, uint32_t, V>;

    template <typename K, typename V>
    using kivtable64 = kivtable<K, uint64_t, V>;
    */

}

