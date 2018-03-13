/**
* Represents a forest of balanced N-ary trees.
*
* Trees are compressed, so identical branches are represented by indices
* to the same location in memory. Empty branches occupy no memory at all.
*
* The leaves are of (hashable) type LK, but we can also hang information
* of type LV from a leaf and NV from a non-leaf. Since identical branches
* occupy the same memory, you cannot hang distinct information from
* identical branches.
*
* I should be an unsigned integer type (usually uint32_t, but this would
* need to be uint64_t if the forest occupies more than 4 billion nodes*).
* Note that using uint64_t instead of uint32_t would result in the forest
* occupying twice as much memory, so this is discouraged unless you do
* actually need more than 4 billion nodes -- unlikely unless you have at
* least 200 gigabytes of RAM.
*
* *Nodes in each layer have separate sets of indices, so you can have up
* to 4 billion nodes in each layer.
*/

#pragma once

#include "nicearray.h"
#include "kivtable.h"
#include <stdint.h>
#include <cstdarg>
#include <string>
#include <map>

namespace apg {

    template<typename I>
    struct hypernode {
        I index;
        uint32_t depth;

        hypernode(I index, uint32_t depth) {
            this->index = index;
            this->depth = depth;
        }

        hypernode() {
            this->index = -1;
            this->depth = 0;
        }
    };

    template<typename I, int N, typename NV, typename LK, typename LV>
    class hypertree {

        // We store a kivtable for each layer in our hypertree:
        std::vector<kivtable<nicearray<I, N>, I, NV>* > nonleaves;
        kivtable<LK, I, LV> leaves;

        public:

        uint64_t total_bytes() {
            uint64_t n = leaves.total_bytes();
            for (unsigned int i = 0; i < nonleaves.size(); i++) {
                n += nonleaves[i]->total_bytes();
            }
            return n;
        }

        // Maps symbol to a node in the hypertree:
        uint64_t hcounter;
        std::map<uint64_t, hypernode<I> > ihandles;
        std::map<std::string, hypernode<I> > handles;

        // Wrapper for nonleaves.ind2ptr:
        kiventry<nicearray<I, N>, I, NV>* ind2ptr_nonleaf(uint32_t depth, I index) {
            return nonleaves[depth-1]->ind2ptr(index);
        }

        // Wrapper for leaves.ind2ptr:
        kiventry<LK, I, LV>* ind2ptr_leaf(I index) {
            return leaves.ind2ptr(index);
        }

        // Get the nth child of a particular node:
        hypernode<I> getchild(hypernode<I> parent, uint32_t n) {
            if (parent.depth == 0 || parent.index == ((I) -1) || n >= N) {
                // Invalid node:
                return hypernode<I>(-1, 0);
            } else {
                I index = parent.index ? ind2ptr_nonleaf(parent.depth, parent.index)->key.x[n] : 0;
                // A child has depth one less than that of its parent:
                return hypernode<I>(index, parent.depth - 1);
            }
        }

        void gc_traverse(bool destructive) {
            /*
            * Run gc_traverse(false) to zero all gcflags;
            * Run gc_traverse(true) to delete all items with zero gcflags.
            */
            for (unsigned int i = 0; i < nonleaves.size(); i++) {
                nonleaves[i]->gc_traverse(destructive);
            }
            leaves.gc_traverse(destructive);
        }

        // Recursively mark node to rescue it from garbage-collection:
        I gc_mark(hypernode<I> parent) {
            if (parent.index == 0 || parent.index == ((I) -1)) {
                return 0;
            } else if (parent.depth == 0) {
                kiventry<LK, I, LV>* pptr = leaves.ind2ptr(parent.index);
                if (pptr->gcflags == 0) {
                    // if (outfile) {(*outfile) << 'L' << ':' << pptr->key.toBase85() << '\n';}
                    pptr->gcflags = (++leaves.gccounter);
                }
                return pptr->gcflags;
            } else {
                kiventry<nicearray<I, N>, I, NV>* pptr = ind2ptr_nonleaf(parent.depth, parent.index);
                if (pptr->gcflags == 0) {
                    nicearray<I, N> children;
                    for (int i = 0; i < N; i++) {
                        children.x[i] = gc_mark(hypernode<I>(pptr->key.x[i], parent.depth-1));
                    }
                    // if (outfile) {(*outfile) << 'N' << parent.depth << ':' << children.toBase85() << '\n';}
                    pptr->gcflags = (++(nonleaves[parent.depth-1]->gccounter));
                }
                return pptr->gcflags;
            }
        }

        void gc_full() {
            gc_traverse(false);
            for (typename std::map<std::string, hypernode<I> >::iterator it = handles.begin(); it != handles.end(); ++it) {
                gc_mark(it->second);
            }
            for (typename std::map<uint64_t, hypernode<I> >::iterator it = ihandles.begin(); it != ihandles.end(); ++it) {
                gc_mark(it->second);
            }
            gc_traverse(true);
        }

        I make_leaf(LK contents) {
            return leaves.getnode(contents, true);
        }

        I make_nonleaf(uint32_t depth, nicearray<I, N> indices) {
            while (nonleaves.size() < depth) {
                // std::cout << "Adding layer " << (nonleaves.size() + 1) << "..." << std::endl;
                nonleaves.push_back(new kivtable<nicearray<I, N>, I, NV>);
                // std::cout << "...done!" << std::endl;
            }
            // std::cout << depth << std::endl;
            return nonleaves[depth-1]->getnode(indices, true);
        }

        hypernode<I> make_nonleaf_hn(uint32_t depth, nicearray<I, N> indices) {
            return hypernode<I>(make_nonleaf(depth, indices), depth);
        }

        hypertree() { hcounter = 0; }

        ~hypertree() {
            // std::cout << "Deleting nonleaves..." << std::endl;
            while (nonleaves.size()) {
                kivtable<nicearray<I, N>, I, NV>* lastktable = nonleaves.back();
                delete lastktable;
                nonleaves.pop_back();
            }
            // std::cout << "...done!" << std::endl;
        }

    };

}

