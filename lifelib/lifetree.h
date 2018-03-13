#pragma once

#include "avxlife/uli.h"
#include "lifetree_abstract.h"
#include "bitbounds.h"
#include "bitworld.h"
#include "sanirule.h"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

namespace apg {

    template<typename I, int N>
    class lifetree : public lifetree_abstract<I> {

        public:
        hypertree<I, 4, lifemeta<I>, nicearray<uint64_t, 4*N>, lifemeta<I> > htree;

        uint64_t countlayers() {
            return N;
        }

        uint64_t newihandle(hypernode<I> hnode) {
            uint64_t x = ++htree.hcounter;
            htree.ihandles.emplace(x, hnode);
            return x;
        }
        void sethandle(uint64_t ihandle, hypernode<I> hnode) {
            htree.ihandles[ihandle] = hnode;
        }
        void sethandle(std::string handle, hypernode<I> hnode) {
            htree.handles[handle] = hnode;
        }
        void delhandle(uint64_t ihandle) {
            htree.ihandles.erase(ihandle);
        }
        void delhandle(std::string handle) {
            htree.handles.erase(handle);
        }
        hypernode<I> gethandle(uint64_t ihandle) {
            return htree.ihandles[ihandle];
        }
        hypernode<I> gethandle(std::string handle) {
            return htree.handles[handle];
        }

        uint64_t total_bytes() { return htree.total_bytes(); }
        void force_gc() { htree.gc_full(); }
        bool threshold_gc(uint64_t threshold) {
            if (threshold) {
                uint64_t oldsize = htree.total_bytes();
                if (oldsize >= threshold) {
                    // std::cerr << "Performing garbage collection (" << oldsize << " >= " << threshold << ")" << std::endl;
                    htree.gc_full();
                    // std::cerr << "Size reduced from " << oldsize << " to " << htree.total_bytes() << " bytes." << std::endl;
                    return true;
                }
            }
            return false;
        }

        uint64_t counthandles() {
            return htree.ihandles.size() + htree.handles.size();
        }

        kiventry<nicearray<I, 4>, I, lifemeta<I> >* ind2ptr_nonleaf(uint32_t depth, I index) {
            return htree.ind2ptr_nonleaf(depth, index);
        }
        kiventry<nicearray<uint64_t, 4*N>, I, lifemeta<I> >* ind2ptr_leaf(I index) {
            return htree.ind2ptr_leaf(index);
        }
        I make_nonleaf(uint32_t depth, nicearray<I, 4> contents) {
            return htree.make_nonleaf(depth, contents);
        }
        hypernode<I> make_nonleaf_hn(uint32_t depth, nicearray<I, 4> contents) {
            return htree.make_nonleaf_hn(depth, contents);
        }
        I make_leaf(nicearray<uint64_t, 4*N> contents) {
            return htree.make_leaf(contents);
        }
        hypernode<I> getchild(hypernode<I> parent, uint32_t n) {
            return htree.getchild(parent, n);
        }
        int rulestring_to_integer(std::string rulestring) {
            return rule2int(rulestring);
        }

        uint64_t leafpart(I index, uint32_t part) {
            kiventry<nicearray<uint64_t, 4*N>, I, lifemeta<I> >* pptr = ind2ptr_leaf(index);
            if (part < 4*N) { return pptr->key.x[part]; } else { return 0; }
        }

        hypernode<I> iterate_recurse(hypernode<I> hnode, uint64_t mantissa, uint64_t exponent, int rule, int history) {
            /*
            * Given a 2^n-by-2^n square represented by a hypernode, return the
            * central 2^(n-1)-by-2^(n-1) subsquare advanced by M * (2 ** E)
            * generations.
            *
            * This uses Gosper's HashLife algorithm down to a base-case where
            * n = 5 (i.e. computing the 16-by-16 interior of a 32-by-32 grid)
            * is performed by vectorised bitsliced assembly code.
            */

            // std::cerr << "Calling iterate_recurse((" << hnode.index << ", " << hnode.depth << "), ";
            // std::cerr << mantissa << ", " << exponent << ", " << rule << ", " << history << ")" << std::endl;

            if (hnode.index == 0) {

                // Node is empty; return an empty node of the next size down:
                return hypernode<I>(0, hnode.depth - 1);

            }

            // Extract the pointer to the node:
            kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr = ind2ptr_nonleaf(hnode.depth, hnode.index);

            // Determine whether 1 or 2 stages are necessary:
            bool bothstages = (hnode.depth <= (1 + exponent));

            // Return the result if we've previously cached it:
            uint64_t gcdesc = pptr->gcflags >> 9;
            uint64_t hrule = (rule << 1) + (history & 1);
            if ((gcdesc & 7) == (mantissa - 1) && (hrule == ((gcdesc >> 3) & 15))) {
                uint64_t gcexp = gcdesc >> 7;
                if (gcexp == (1 + exponent) || (bothstages && (gcexp >= hnode.depth))) {
                    // The exponent and mantissa are compatible with their desired values:
                    return hypernode<I>(pptr->value.res, hnode.depth - 1);
                }
            }

            if (hnode.depth == 1) {

                // Set up the memory locations:
                nicearray<uint64_t, 4*N> outleaf = {0ull};

                uint64_t* inleafxs[4];

                for (int i = 0; i < 4; i++) {
                    inleafxs[i] = ind2ptr_leaf(pptr->key.x[i])->key.x;
                }

                universal_leaf_iterator<N>(rule, history, mantissa, inleafxs, outleaf.x);

                I finalnode = make_leaf(outleaf);

                if (mantissa != 0) {
                    // Cache the result to save additional recomputation:
                    pptr->value.res = finalnode;
                    uint64_t new_gcdesc = ((1 + exponent) << 7) | (hrule << 3) | (mantissa - 1);
                    pptr->gcflags = (pptr->gcflags & 511) | (new_gcdesc << 9);
                }

                // Return the result:
                return hypernode<I>(finalnode, 0);

            } else {

                // Extract the pointers for the children:
                kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr_tl = ind2ptr_nonleaf(hnode.depth-1, pptr->key.x[0]);
                kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr_tr = ind2ptr_nonleaf(hnode.depth-1, pptr->key.x[1]);
                kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr_bl = ind2ptr_nonleaf(hnode.depth-1, pptr->key.x[2]);
                kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr_br = ind2ptr_nonleaf(hnode.depth-1, pptr->key.x[3]);

                // Determine the centre square and return if (mantissa == 0):
                nicearray<I, 4> cc = {pptr_tl->key.x[3], pptr_tr->key.x[2], pptr_bl->key.x[1], pptr_br->key.x[0]};
                hypernode<I>  hncc = make_nonleaf_hn(hnode.depth-1, cc);
                if (mantissa == 0) { return hncc; }

                // Actual HashLife algorithm begins here:
                nicearray<I, 4> tc = {pptr_tl->key.x[1], pptr_tr->key.x[0], pptr_tl->key.x[3], pptr_tr->key.x[2]};
                nicearray<I, 4> bc = {pptr_bl->key.x[1], pptr_br->key.x[0], pptr_bl->key.x[3], pptr_br->key.x[2]};
                nicearray<I, 4> cl = {pptr_tl->key.x[2], pptr_tl->key.x[3], pptr_bl->key.x[0], pptr_bl->key.x[1]};
                nicearray<I, 4> cr = {pptr_tr->key.x[2], pptr_tr->key.x[3], pptr_br->key.x[0], pptr_br->key.x[1]};

                // Compute the nine subnodes after the first stage:
                uint64_t newmant = bothstages ? mantissa : 0;
                hypernode<I>   xcc = iterate_recurse(hncc, newmant, exponent, rule, history);
                hypernode<I>   xtc = iterate_recurse(make_nonleaf_hn(hnode.depth - 1, tc), newmant, exponent, rule, history);
                hypernode<I>   xbc = iterate_recurse(make_nonleaf_hn(hnode.depth - 1, bc), newmant, exponent, rule, history);
                hypernode<I>   xcl = iterate_recurse(make_nonleaf_hn(hnode.depth - 1, cl), newmant, exponent, rule, history);
                hypernode<I>   xcr = iterate_recurse(make_nonleaf_hn(hnode.depth - 1, cr), newmant, exponent, rule, history);
                hypernode<I>   xtl = iterate_recurse(hypernode<I>(pptr->key.x[0], hnode.depth - 1), newmant, exponent, rule, history);
                hypernode<I>   xtr = iterate_recurse(hypernode<I>(pptr->key.x[1], hnode.depth - 1), newmant, exponent, rule, history);
                hypernode<I>   xbl = iterate_recurse(hypernode<I>(pptr->key.x[2], hnode.depth - 1), newmant, exponent, rule, history);
                hypernode<I>   xbr = iterate_recurse(hypernode<I>(pptr->key.x[3], hnode.depth - 1), newmant, exponent, rule, history);

                // Compute the four subnodes after the second stage:
                nicearray<I, 4> tl = {xtl.index, xtc.index, xcl.index, xcc.index};
                nicearray<I, 4> tr = {xtc.index, xtr.index, xcc.index, xcr.index};
                nicearray<I, 4> bl = {xcl.index, xcc.index, xbl.index, xbc.index};
                nicearray<I, 4> br = {xcc.index, xcr.index, xbc.index, xbr.index};
                hypernode<I>   ytl = iterate_recurse(make_nonleaf_hn(hnode.depth - 1, tl), mantissa, exponent, rule, history);
                hypernode<I>   ytr = iterate_recurse(make_nonleaf_hn(hnode.depth - 1, tr), mantissa, exponent, rule, history);
                hypernode<I>   ybl = iterate_recurse(make_nonleaf_hn(hnode.depth - 1, bl), mantissa, exponent, rule, history);
                hypernode<I>   ybr = iterate_recurse(make_nonleaf_hn(hnode.depth - 1, br), mantissa, exponent, rule, history);

                // Assemble the four subnodes and calculate the result:
                nicearray<I, 4> y = {ytl.index, ytr.index, ybl.index, ybr.index};
                I finalnode = make_nonleaf(hnode.depth - 1, y);

                // Cache the result to save additional recomputation:
                pptr->value.res = finalnode;
                uint64_t new_gcdesc = ((1 + exponent) << 7) | (hrule << 3) | (mantissa - 1);
                pptr->gcflags = (pptr->gcflags & 511) | (new_gcdesc << 9);

                // Return the result:
                return hypernode<I>(finalnode, hnode.depth - 1);
            }

        }

        uint64_t write_macrocell_leaf(std::ostream &outstream, uint64_t leaf,
                                     std::map<uint64_t, uint64_t> *subleaf2int,
                                     uint64_t &linenum) {

            auto it = subleaf2int->find(leaf);
            if (leaf == 0) {
                return 0;
            } else if (it != subleaf2int->end()) {
                return it->second;
            } else {
                uint64_t x = leaf;
                for (int i = 0; i < 8; i++) {
                    for (int j = 0; j < 8; j++) {
                        outstream << ".*"[x & 1];
                        x = x >> 1;
                    }
                    outstream << "$";
                }
                outstream << std::endl;
                subleaf2int->emplace(leaf, (++linenum));
                return linenum;
            }
        }

        uint64_t write_macrocell_recurse(std::ostream &outstream, hypernode<I> hnode,
                                     std::map<uint64_t, uint64_t> *subleaf2int,
                                     std::map<std::pair<I, uint32_t>, uint64_t> *hnode2int,
                                     uint64_t &linenum) {
            /*
            * Writes a 2-state macrocell file according to the contents of
            * layer 0.
            */
            auto it = hnode2int->find(std::make_pair(hnode.index, hnode.depth));

            if (hnode.index == 0) {
                return 0;
            } else if (it != hnode2int->end()) {
                return it->second;
            } else if (hnode.depth == 0) {
                // Extract the pointer to the node:
                kiventry<nicearray<uint64_t, 4*N>, I, lifemeta<I> >* pptr = ind2ptr_leaf(hnode.index);
                uint64_t a = write_macrocell_leaf(outstream, pptr->key.x[0], subleaf2int, linenum);
                uint64_t b = write_macrocell_leaf(outstream, pptr->key.x[1], subleaf2int, linenum);
                uint64_t c = write_macrocell_leaf(outstream, pptr->key.x[2], subleaf2int, linenum);
                uint64_t d = write_macrocell_leaf(outstream, pptr->key.x[3], subleaf2int, linenum);
                outstream << (hnode.depth + 4) << " " << a << " " << b << " " << c << " " << d << std::endl;
                hnode2int->emplace(std::make_pair(hnode.index, hnode.depth), (++linenum));
                return linenum;
            } else {
                uint64_t a = write_macrocell_recurse(outstream, getchild(hnode, 0), subleaf2int, hnode2int, linenum);
                uint64_t b = write_macrocell_recurse(outstream, getchild(hnode, 1), subleaf2int, hnode2int, linenum);
                uint64_t c = write_macrocell_recurse(outstream, getchild(hnode, 2), subleaf2int, hnode2int, linenum);
                uint64_t d = write_macrocell_recurse(outstream, getchild(hnode, 3), subleaf2int, hnode2int, linenum);
                outstream << (hnode.depth + 4) << " " << a << " " << b << " " << c << " " << d << std::endl;
                hnode2int->emplace(std::make_pair(hnode.index, hnode.depth), (++linenum));
                return linenum;
            }
        }

        void write_macrocell(std::ostream &outstream, hypernode<I> hnode, std::string rule) {
            outstream << "[M2] (lifelib " << LIFELIB_VERSION << ")" << std::endl;
            if (rule != "") { outstream << "#R " << gollyrule(rule) << std::endl; }
            std::map<uint64_t, uint64_t> subleaf2int;
            std::map<std::pair<I, uint32_t>, uint64_t> hnode2int;
            uint64_t linenum = 0;
            write_macrocell_recurse(outstream, hnode, &subleaf2int, &hnode2int, linenum);
        }

        hypernode<I> read_macrocell(std::istream &instream, std::map<uint64_t, uint64_t> *lmap, std::string &rule) {
            /*
            * Returns a hypernode representing the contents of a macrocell
            * file. This handles both 2- and n-state macrocell files using the
            * same code.
            *
            * lmap should be a pointer to a map which translates the states
            * given in the macrocell file to a bit-array in memory. For
            * instance, if state 7 should map to 'on' in layers 0, 3 and 4,
            * then the map should have:
            *
            * (*lmap)[7] = 0b00011001 = 25
            *
            * Zero must always map to zero. If lmap is a null pointer, then it
            * is taken to be the identity map.
            */

            std::string line; // line read from file
            std::vector<nicearray<uint64_t, N> > pleaves; // partial leaves
            nicearray<uint64_t, N> zeropleaf = {0ull}; // empty partial leaf
            pleaves.push_back(zeropleaf); // zero means zero

            uint32_t log2size = 0; // log2(size) of most recent node
            I lastnode = -1; // index of most recent node

            while (std::getline(instream, line)) {
                // std::cout << line << std::endl;
                if (line.empty()) {
                    continue;
                } else if (line[0] == '[') {
                    // header line
                    // std::cerr << "Header line: " << line << std::endl;
                } else if (line[0] == '#') {
                    // std::cerr << "Comment line: " << line << std::endl;
                    if (line.length() >= 2 && (line[1] == 'R' || line[1] == 'G')) {
                        if (line[1] == 'R') {
                            rule = sanirule(line.substr(3));
                            std::cerr << line << " interpreted as " << rule << std::endl;
                        }
                    } else {
                        // line is a comment
                    }
                } else {
                    nicearray<uint64_t, N> pleaf = {0ull};
                    if (line[0] == '.' || line[0] == '*' || line[0] == '$') {
                        uint64_t lm = (lmap == 0) ? 1 : ((*lmap)[1]);
                        uint64_t pl = 0;
                        uint64_t x = 0;
                        uint64_t y = 0;
                        // Load the 8-by-8 pixel representation into uint64_t pl:
                        for (unsigned int i = 0; i < line.length(); i++) {
                            if (line[i] == '$') {
                                x = 0;
                                y += 1;
                            } else {
                                if (line[i] == '*') {
                                    pl |= (1ull << (x + 8*y));
                                }
                                x += 1;
                            }
                        }
                        if (pl == 0) {std::cerr << "Warning: " << line << std::endl;}
                        // std::cout << pl << std::endl;
                        // Populate the partial leaf according to the leafmap:
                        for (unsigned int i = 0; i < N; i++) {
                            if (lm & 1) {
                                pleaf.x[i] = pl;
                            } else {
                                pleaf.x[i] = 0;
                            }
                            lm = lm >> 1;
                        }
                    } else if (line[0] >= '1' && line[0] <= '9') {

                        // Line should be a space-separated list of 5 integers:
                        std::stringstream s(line);
                        uint64_t a = 0, b = 0, c = 0, d = 0;
                        s >> log2size >> a >> b >> c >> d;

                        if (log2size == 1) {
                            uint64_t ilma = (lmap == 0) ? a : ((*lmap)[a]);
                            uint64_t ilmb = (lmap == 0) ? b : ((*lmap)[b]);
                            uint64_t ilmc = (lmap == 0) ? c : ((*lmap)[c]);
                            uint64_t ilmd = (lmap == 0) ? d : ((*lmap)[d]);
                            for (unsigned int i = 0; i < N; i++) {
                                pleaf.x[i] = (ilma & 1) | ((ilmb & 1) << 1) | ((ilmc & 1) << 8) | ((ilmd & 1) << 9);
                                ilma = ilma >> 1; ilmb = ilmb >> 1; ilmc = ilmc >> 1; ilmd = ilmd >> 1;
                            }
                        } else if (log2size == 2) {
                            for (unsigned int i = 0; i < N; i++) {
                                pleaf.x[i]  =  pleaves[a].x[i];
                                pleaf.x[i] |= (pleaves[b].x[i] << 2);
                                pleaf.x[i] |= (pleaves[c].x[i] << 16);
                                pleaf.x[i] |= (pleaves[d].x[i] << 18);
                            }
                        } else if (log2size == 3) {
                            for (unsigned int i = 0; i < N; i++) {
                                pleaf.x[i]  =  pleaves[a].x[i];
                                pleaf.x[i] |= (pleaves[b].x[i] << 4);
                                pleaf.x[i] |= (pleaves[c].x[i] << 32);
                                pleaf.x[i] |= (pleaves[d].x[i] << 36);
                            }
                        } else if (log2size == 4) {
                            // Leaf:
                            nicearray<uint64_t, 4*N> leaf;
                            for (unsigned int i = 0; i < N; i++) {
                                leaf.x[4*i]   = pleaves[a].x[i];
                                leaf.x[4*i+1] = pleaves[b].x[i];
                                leaf.x[4*i+2] = pleaves[c].x[i];
                                leaf.x[4*i+3] = pleaves[d].x[i];
                            }
                            lastnode = make_leaf(leaf);
                            pleaf.x[0] = lastnode;
                        } else {
                            // Nonleaf:
                            I tl = pleaves[a].x[0];
                            I tr = pleaves[b].x[0];
                            I bl = pleaves[c].x[0];
                            I br = pleaves[d].x[0];
                            nicearray<I, 4> nonleaf = {tl, tr, bl, br};
                            lastnode = make_nonleaf(log2size - 4, nonleaf);
                            pleaf.x[0] = lastnode;
                        }
                    } else {
                        std::cerr << "Invalid line: " << line << std::endl;
                        continue;
                    }
                    pleaves.push_back(pleaf);
                }
            }

            return hypernode<I>(lastnode, log2size - 4);
        }

        // Extract the (x, y)th cell from a node:
        uint64_t getcell_recurse(hypernode<I> hnode, uint64_t x, uint64_t y) {
            if (hnode.index == 0) {
                return 0;
            } else if (hnode.depth == 0) {
                kiventry<nicearray<uint64_t, 4*N>, I, lifemeta<I> >* pptr = ind2ptr_leaf(hnode.index);
                uint64_t c = 0;
                for (unsigned int i = 0; i < N; i++) {
                    uint64_t rel = pptr->key.x[4*i + ((y & 8) >> 2) + ((x & 8) >> 3)];
                    rel = rel >> (x & 7);
                    rel = rel >> ((y & 7) << 3);
                    c |= ((rel & 1) << i);
                }
                return c;
            } else {
                uint64_t tx = (x >> (hnode.depth + 3)) & 1;
                uint64_t ty = (y >> (hnode.depth + 3)) & 1;
                return getcell_recurse(getchild(hnode, tx + 2*ty), x, y);
            }
        }

        I getpop_recurse(hypernode<I> hnode, I modprime, uint64_t layermask) {
            /*
            * Compute the population mod p of a given hypernode. A cell of
            * state S is considered alive if and only if (layermask & S) != 0
            * where & indicates bitwise conjunction. Equivalently, we take the
            * population of the union of the layers indexed by the bits in
            * layermask.
            *
            * If the layermask is changed, you should run a garbage-collection
            * so as to clear the memoized population counts.
            */

            if (hnode.index == 0) {

                // Empty nodes have population 0:
                return 0;

            } else if (hnode.depth == 0) {

                // This is a leaf node (16-by-16 square); extract its memory location:
                kiventry<nicearray<uint64_t, 4*N>, I, lifemeta<I> >* pptr = ind2ptr_leaf(hnode.index);

                if (pptr->gcflags & 1) {
                    // We've cached the population; return it:
                    return pptr->value.aux;
                } else {
                    // Accumulate the popcount of the four 8-by-8 subleaves:
                    I pop = 0;
                    uint64_t a = 0, b = 0, c = 0, d = 0;
                    for (unsigned int i = 0; i < N; i++) {
                        if (layermask & (1ull << i)) {
                            a |= pptr->key.x[4*i];
                            b |= pptr->key.x[4*i+1];
                            c |= pptr->key.x[4*i+2];
                            d |= pptr->key.x[4*i+3];
                        }
                    }
                    pop += __builtin_popcountll(a);
                    pop += __builtin_popcountll(b);
                    pop += __builtin_popcountll(c);
                    pop += __builtin_popcountll(d);
                    pptr->value.aux = pop;
                    pptr->gcflags |= 1;
                    return pop;
                }

            } else {

                // Non-leaf node (32-by-32 or larger):
                kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr = ind2ptr_nonleaf(hnode.depth, hnode.index);
                I oldflags = pptr->gcflags;

                // Determine whether our cached value for the population is correct.
                // If the depth is <= 11 (i.e. 32768-by-32768 or smaller), then the
                // population is smaller than any of the prime moduli we ever use
                // (namely 2^30 + k):
                bool goodpop = (hnode.depth <= 11) ? (oldflags & 1) : (((oldflags ^ modprime) & 0x1ff) == 0);

                if (goodpop) {
                    // Return cached result:
                    return pptr->value.aux;
                } else {
                    // Recompute population recursively:
                    I pop = 0;
                    for (int i = 0; i < 4; i++) {
                        pop += getpop_recurse(hypernode<I>(pptr->key.x[i], hnode.depth-1), modprime, layermask);
                        pop %= modprime;
                    }
                    pptr->value.aux = pop;
                    oldflags ^= (oldflags & 0x1ff);
                    oldflags |= (modprime & 0x1ff);
                    pptr->gcflags = oldflags;
                    return pop;
                }
            }
        }

        hypernode<I> subnode(hypernode<I> hnode, uint64_t x, uint64_t y, uint64_t n) {
            hypernode<I> hnode2 = hnode;
            uint64_t i = n;
            while (i --> 0) {
                uint64_t tx = (x >> i) & 1;
                uint64_t ty = (y >> i) & 1;
                hnode2 = getchild(hnode2, tx + 2 * ty);
            }
            return hnode2;
        }

        uint64_t bound_recurse(hypernode<I> hnode, int direction, std::map<std::pair<I, uint32_t>, uint64_t> *memmap) {

            uint64_t z = (direction & 2) ? 0 : (16 << hnode.depth);
            auto it = memmap->find(std::make_pair(hnode.index, hnode.depth));

            if (it != memmap->end()) {
                return it->second;
            } else if (hnode.depth == 0) {
                kiventry<nicearray<uint64_t, 4*N>, I, lifemeta<I> >* pptr = ind2ptr_leaf(hnode.index);
                uint64_t tilea = 0;
                uint64_t tileb = 0;
                if (direction & 1) {
                    for (int i = 0; i < N; i++) {
                        tilea |= pptr->key.x[4*i];
                        tilea |= pptr->key.x[4*i+1];
                        tileb |= pptr->key.x[4*i+2];
                        tileb |= pptr->key.x[4*i+3];
                    }
                    if (direction & 2) {
                        return (tileb ? (8 + uint64_bottom(tileb)) : uint64_bottom(tilea));
                    } else {
                        return (tilea ? uint64_top(tilea) : (8 + uint64_top(tileb)));
                    }
                } else {
                    for (int i = 0; i < N; i++) {
                        tilea |= pptr->key.x[4*i];
                        tileb |= pptr->key.x[4*i+1];
                        tilea |= pptr->key.x[4*i+2];
                        tileb |= pptr->key.x[4*i+3];
                    }
                    if (direction & 2) {
                        return (tileb ? (8 + uint64_right(tileb)) : uint64_right(tilea));
                    } else {
                        return (tilea ? uint64_left(tilea) : (8 + uint64_left(tileb)));
                    }
                }
            } else {
                bool nonzero = false;
                uint64_t n = (hnode.depth < 4) ? hnode.depth : 4;
                int64_t nexp = (1ull << n) - 1;
                if (direction == 0) {
                    for (int64_t x = 0; x <= nexp; x++) {
                        for (int64_t y = 0; y <= nexp; y++) {
                            hypernode<I> hnode2 = subnode(hnode, x, y, n);
                            if (hnode2.index) {
                                nonzero = true;
                                uint64_t a = bound_recurse(hnode2, direction, memmap) + (x << (hnode2.depth + 4));
                                z = (a < z) ? a : z;
                            }
                        }
                        if (nonzero) {
                            memmap->emplace(std::make_pair(hnode.index, hnode.depth), z);
                            return z;
                        }
                    }
                } else if (direction == 1) {
                    for (int64_t y = 0; y <= nexp; y++) {
                        for (int64_t x = 0; x <= nexp; x++) {
                            hypernode<I> hnode2 = subnode(hnode, x, y, n);
                            if (hnode2.index) {
                                nonzero = true;
                                uint64_t a = bound_recurse(hnode2, direction, memmap) + (y << (hnode2.depth + 4));
                                z = (a < z) ? a : z;
                            }
                        }
                        if (nonzero) {
                            memmap->emplace(std::make_pair(hnode.index, hnode.depth), z);
                            return z;
                        }
                    }
                } else if (direction == 2) {
                    for (int64_t x = nexp; x >= 0; x--) {
                        for (int64_t y = 0; y <= nexp; y++) {
                            hypernode<I> hnode2 = subnode(hnode, x, y, n);
                            if (hnode2.index) {
                                nonzero = true;
                                uint64_t a = bound_recurse(hnode2, direction, memmap) + (x << (hnode2.depth + 4));
                                z = (a > z) ? a : z;
                            }
                        }
                        if (nonzero) {
                            memmap->emplace(std::make_pair(hnode.index, hnode.depth), z);
                            return z;
                        }
                    }
                } else if (direction == 3) {
                    for (int64_t y = nexp; y >= 0; y--) {
                        for (int64_t x = 0; x <= nexp; x++) {
                            hypernode<I> hnode2 = subnode(hnode, x, y, n);
                            if (hnode2.index) {
                                nonzero = true;
                                uint64_t a = bound_recurse(hnode2, direction, memmap) + (y << (hnode2.depth + 4));
                                z = (a > z) ? a : z;
                            }
                        }
                        if (nonzero) {
                            memmap->emplace(std::make_pair(hnode.index, hnode.depth), z);
                            return z;
                        }
                    }
                }
            }
            return z;
        }

        hypernode<I> tensor_recurse(hypernode<I> hnode, lifetree_abstract<I> *lab, uint32_t delta, std::vector<I> &v,
                                    std::map<std::pair<I, uint32_t>, I> *memmap) {
            /*
            * Recursively copy a structure from one lifetree to another,
            * where individual cells in the reference pattern are mapped
            * to entire 2^delta-by-2^delta 'metacells' in this lifetree.
            */

            auto it = memmap->find(std::make_pair(hnode.index, hnode.depth));

            if (hnode.index == 0) {
                return hypernode<I>(0, hnode.depth + delta);
            } else if (it != memmap->end()) {
                return hypernode<I>(it->second, hnode.depth + delta);
            } else if (hnode.depth == 0) {
                I partials[16][16];
                uint64_t modulus = v.size();
                for (int i = 0; i < 16; i++) {
                    for (int j = 0; j < 16; j++) {
                        partials[i][j] = v[lab->getcell_recurse(hnode, i, j) % modulus];
                    }
                }
                for (int k = 3; k >= 0; k--) {
                    for (int i = 0; i < (1 << k); i++) {
                        for (int j = 0; j < (1 << k); j++) {
                            nicearray<I, 4> cc = {partials[2*i][2*j], partials[2*i+1][2*j], partials[2*i][2*j+1], partials[2*i+1][2*j+1]};
                            partials[i][j] = make_nonleaf(delta - k, cc);
                        }
                    }
                }
                memmap->emplace(std::make_pair(hnode.index, hnode.depth), partials[0][0]);
                return hypernode<I>(partials[0][0], delta);
            } else {
                hypernode<I> ytl = tensor_recurse(lab->getchild(hnode, 0), lab, delta, v, memmap);
                hypernode<I> ytr = tensor_recurse(lab->getchild(hnode, 1), lab, delta, v, memmap);
                hypernode<I> ybl = tensor_recurse(lab->getchild(hnode, 2), lab, delta, v, memmap);
                hypernode<I> ybr = tensor_recurse(lab->getchild(hnode, 3), lab, delta, v, memmap);

                // Assemble the transformed node:
                nicearray<I, 4> cc = {ytl.index, ytr.index, ybl.index, ybr.index};
                hypernode<I> xcc = make_nonleaf_hn(hnode.depth + delta, cc);
                memmap->emplace(std::make_pair(hnode.index, hnode.depth), xcc.index);
                return xcc;
            }

        }

        hypernode<I> copy_recurse(hypernode<I> hnode, lifetree_abstract<I> *lab, std::map<std::pair<I, uint32_t>, I> *memmap) {
            /*
            * Recursively copy a structure from one lifetree to another.
            */

            auto it = memmap->find(std::make_pair(hnode.index, hnode.depth));

            if (hnode.index == 0) {
                return hypernode<I>(0, hnode.depth);
            } else if (it != memmap->end()) {
                return hypernode<I>(it->second, hnode.depth);
            } else if (hnode.depth == 0) {
                nicearray<uint64_t, 4*N> outleaf = {0ull};
                for (int i = 0; i < 4*N; i++) {
                    outleaf.x[i] = lab->leafpart(hnode.index, i);
                }
                I res = make_leaf(outleaf);
                memmap->emplace(std::make_pair(hnode.index, 0), res);
                return hypernode<I>(res, 0);
            } else {
                hypernode<I> ytl = copy_recurse(lab->getchild(hnode, 0), lab, memmap);
                hypernode<I> ytr = copy_recurse(lab->getchild(hnode, 1), lab, memmap);
                hypernode<I> ybl = copy_recurse(lab->getchild(hnode, 2), lab, memmap);
                hypernode<I> ybr = copy_recurse(lab->getchild(hnode, 3), lab, memmap);

                // Assemble the transformed node:
                nicearray<I, 4> cc = {ytl.index, ytr.index, ybl.index, ybr.index};
                hypernode<I> xcc = make_nonleaf_hn(hnode.depth, cc);
                memmap->emplace(std::make_pair(hnode.index, hnode.depth), xcc.index);
                return xcc;
            }
        }

        hypernode<I> brand_recurse(hypernode<I> hnode, std::map<std::pair<I, uint32_t>, I> *memmap) {

            if (N == 1) { return hnode; }

            auto it = memmap->find(std::make_pair(hnode.index, hnode.depth));

            if (hnode.index == 0) {
                return hypernode<I>(0, hnode.depth);
            } else if (it != memmap->end()) {
                return hypernode<I>(it->second, hnode.depth);
            } else if (hnode.depth == 0) {
                // Extract the pointer to the node:
                kiventry<nicearray<uint64_t, 4*N>, I, lifemeta<I> >* pptr = ind2ptr_leaf(hnode.index);
                nicearray<uint64_t, 4*N> outleaf = {0ull};

                for (int j = 0; j < 4; j++) {
                    uint64_t l = pptr->key.x[j];
                    for (int i = 1; i < N; i++) { l &= pptr->key.x[4*i+j]; }
                    for (int i = 0; i < N; i++) { outleaf.x[4*i+j] = l; }
                }

                I res = make_leaf(outleaf);
                memmap->emplace(std::make_pair(hnode.index, 0), res);
                return hypernode<I>(res, 0);
            } else {
                // Extract the pointer to the node:
                kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr = ind2ptr_nonleaf(hnode.depth, hnode.index);

                // Lazy evaluation:
                hypernode<I> ytl = brand_recurse(hypernode<I>(pptr->key.x[0], hnode.depth - 1), memmap);
                hypernode<I> ytr = brand_recurse(hypernode<I>(pptr->key.x[1], hnode.depth - 1), memmap);
                hypernode<I> ybl = brand_recurse(hypernode<I>(pptr->key.x[2], hnode.depth - 1), memmap);
                hypernode<I> ybr = brand_recurse(hypernode<I>(pptr->key.x[3], hnode.depth - 1), memmap);

                // Assemble the transformed node:
                nicearray<I, 4> cc = {ytl.index, ytr.index, ybl.index, ybr.index};
                hypernode<I> xcc = make_nonleaf_hn(hnode.depth, cc);
                memmap->emplace(std::make_pair(hnode.index, hnode.depth), xcc.index);
                return xcc;
            }

        }

        hypernode<I> transform_recurse(hypernode<I> hnode, uint8_t perm, std::map<std::pair<I, uint32_t>, I> *memmap) {
            
            auto it = memmap->find(std::make_pair(hnode.index, hnode.depth));

            if (hnode.index == 0) {
                return hypernode<I>(0, hnode.depth);
            } else if (it != memmap->end()) {
                return hypernode<I>(it->second, hnode.depth);
            } else if (hnode.depth == 0) {
                // Extract the pointer to the node:
                kiventry<nicearray<uint64_t, 4*N>, I, lifemeta<I> >* pptr = ind2ptr_leaf(hnode.index);

                nicearray<uint64_t, 4*N> outleaf = {0ull};
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < 4; j++) {
                        outleaf.x[4*i+j] = transform_uint64(pptr->key.x[4*i+((perm >> (2*j)) & 3)], perm);
                    }
                }

                I res = make_leaf(outleaf);
                memmap->emplace(std::make_pair(hnode.index, 0), res);
                return hypernode<I>(res, 0);
            } else {
                // Extract the pointer to the node:
                kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr = ind2ptr_nonleaf(hnode.depth, hnode.index);

                // Lazy evaluation:
                hypernode<I> ytl = transform_recurse(hypernode<I>(pptr->key.x[perm & 3], hnode.depth - 1), perm, memmap);
                hypernode<I> ytr = transform_recurse(hypernode<I>(pptr->key.x[(perm >> 2) & 3], hnode.depth - 1), perm, memmap);
                hypernode<I> ybl = transform_recurse(hypernode<I>(pptr->key.x[(perm >> 4) & 3], hnode.depth - 1), perm, memmap);
                hypernode<I> ybr = transform_recurse(hypernode<I>(pptr->key.x[(perm >> 6) & 3], hnode.depth - 1), perm, memmap);

                // Assemble the transformed node:
                nicearray<I, 4> cc = {ytl.index, ytr.index, ybl.index, ybr.index};
                hypernode<I> xcc = make_nonleaf_hn(hnode.depth, cc);
                memmap->emplace(std::make_pair(hnode.index, hnode.depth), xcc.index);
                return xcc;
            }
        }

        hypernode<I> shift_recurse(hypernode<I> hnode, uint64_t x, uint64_t y, uint64_t exponent, std::map<std::pair<I, uint32_t>, I> *memmap) {

            auto it = memmap->find(std::make_pair(hnode.index, hnode.depth));

            if (hnode.index == 0) {
                return hypernode<I>(0, hnode.depth - 1);
            } else if (it != memmap->end()) {
                return hypernode<I>(it->second, hnode.depth - 1);
            } else {

                // Extract the pointer to the node:
                kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr = ind2ptr_nonleaf(hnode.depth, hnode.index);

                if (hnode.depth + 2 < exponent) {

                    // Shift by zero:
                    I res = pptr->key.x[0];
                    memmap->emplace(std::make_pair(hnode.index, hnode.depth), res);
                    return hypernode<I>(res, hnode.depth - 1);

                } else if (hnode.depth > 1) {

                    // We want to do sign-extended right-shift:
                    uint64_t bs = hnode.depth + 2 - exponent;
                    bs = (bs < 64) ? bs : 63;
                    uint64_t tx = (x >> bs) & 1;
                    uint64_t ty = (y >> bs) & 1;

                    // Extract the pointers for the children:
                    kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr_tl = ind2ptr_nonleaf(hnode.depth-1, pptr->key.x[0]);
                    kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr_tr = ind2ptr_nonleaf(hnode.depth-1, pptr->key.x[1]);
                    kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr_bl = ind2ptr_nonleaf(hnode.depth-1, pptr->key.x[2]);
                    kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr_br = ind2ptr_nonleaf(hnode.depth-1, pptr->key.x[3]);

                    hypernode<I> xtl, xtr, xbl, xbr;

                    if (ty) {
                        if (tx) {
                            nicearray<I, 4> tl2 = {pptr_tl->key.x[3], pptr_tr->key.x[2], pptr_bl->key.x[1], pptr_br->key.x[0]};
                            nicearray<I, 4> tr2 = {pptr_tr->key.x[2], pptr_tr->key.x[3], pptr_br->key.x[0], pptr_br->key.x[1]};
                            nicearray<I, 4> bl2 = {pptr_bl->key.x[1], pptr_br->key.x[0], pptr_bl->key.x[3], pptr_br->key.x[2]};
                            nicearray<I, 4> br2 = {pptr_br->key.x[0], pptr_br->key.x[1], pptr_br->key.x[2], pptr_br->key.x[3]};
                            xtl = make_nonleaf_hn(hnode.depth - 1, tl2);
                            xtr = make_nonleaf_hn(hnode.depth - 1, tr2);
                            xbl = make_nonleaf_hn(hnode.depth - 1, bl2);
                            xbr = make_nonleaf_hn(hnode.depth - 1, br2);
                        } else {
                            nicearray<I, 4> tl2 = {pptr_tl->key.x[2], pptr_tl->key.x[3], pptr_bl->key.x[0], pptr_bl->key.x[1]};
                            nicearray<I, 4> tr2 = {pptr_tl->key.x[3], pptr_tr->key.x[2], pptr_bl->key.x[1], pptr_br->key.x[0]};
                            nicearray<I, 4> bl2 = {pptr_bl->key.x[0], pptr_bl->key.x[1], pptr_bl->key.x[2], pptr_bl->key.x[3]};
                            nicearray<I, 4> br2 = {pptr_bl->key.x[1], pptr_br->key.x[0], pptr_bl->key.x[3], pptr_br->key.x[2]};
                            xtl = make_nonleaf_hn(hnode.depth - 1, tl2);
                            xtr = make_nonleaf_hn(hnode.depth - 1, tr2);
                            xbl = make_nonleaf_hn(hnode.depth - 1, bl2);
                            xbr = make_nonleaf_hn(hnode.depth - 1, br2);
                        }
                    } else {
                        if (tx) {
                            nicearray<I, 4> tl2 = {pptr_tl->key.x[1], pptr_tr->key.x[0], pptr_tl->key.x[3], pptr_tr->key.x[2]};
                            nicearray<I, 4> tr2 = {pptr_tr->key.x[0], pptr_tr->key.x[1], pptr_tr->key.x[2], pptr_tr->key.x[3]};
                            nicearray<I, 4> bl2 = {pptr_tl->key.x[3], pptr_tr->key.x[2], pptr_bl->key.x[1], pptr_br->key.x[0]};
                            nicearray<I, 4> br2 = {pptr_tr->key.x[2], pptr_tr->key.x[3], pptr_br->key.x[0], pptr_br->key.x[1]};
                            xtl = make_nonleaf_hn(hnode.depth - 1, tl2);
                            xtr = make_nonleaf_hn(hnode.depth - 1, tr2);
                            xbl = make_nonleaf_hn(hnode.depth - 1, bl2);
                            xbr = make_nonleaf_hn(hnode.depth - 1, br2);
                        } else {
                            nicearray<I, 4> tl2 = {pptr_tl->key.x[0], pptr_tl->key.x[1], pptr_tl->key.x[2], pptr_tl->key.x[3]};
                            nicearray<I, 4> tr2 = {pptr_tl->key.x[1], pptr_tr->key.x[0], pptr_tl->key.x[3], pptr_tr->key.x[2]};
                            nicearray<I, 4> bl2 = {pptr_tl->key.x[2], pptr_tl->key.x[3], pptr_bl->key.x[0], pptr_bl->key.x[1]};
                            nicearray<I, 4> br2 = {pptr_tl->key.x[3], pptr_tr->key.x[2], pptr_bl->key.x[1], pptr_br->key.x[0]};
                            xtl = make_nonleaf_hn(hnode.depth - 1, tl2);
                            xtr = make_nonleaf_hn(hnode.depth - 1, tr2);
                            xbl = make_nonleaf_hn(hnode.depth - 1, bl2);
                            xbr = make_nonleaf_hn(hnode.depth - 1, br2);
                        }
                    }

                    hypernode<I> ytl = shift_recurse(xtl, x, y, exponent, memmap);
                    hypernode<I> ytr = shift_recurse(xtr, x, y, exponent, memmap);
                    hypernode<I> ybl = shift_recurse(xbl, x, y, exponent, memmap);
                    hypernode<I> ybr = shift_recurse(xbr, x, y, exponent, memmap);
                    nicearray<I, 4> cc2 = {ytl.index, ytr.index, ybl.index, ybr.index};
                    hypernode<I> xcc = make_nonleaf_hn(hnode.depth - 1, cc2);
                    memmap->emplace(std::make_pair(hnode.index, hnode.depth), xcc.index);
                    return xcc;

                } else {

                    // We have a 32-by-32 square:
                    uint64_t tx = (exponent < 4) ? ((x << exponent) & 15) : 0;
                    uint64_t ty = (exponent < 4) ? ((y << exponent) & 15) : 0;
                    nicearray<uint64_t, 4*N> outleaf = {0ull};

                    int bis = apg::best_instruction_set();

                    for (int j = 0; j < N; j++) {
                        uint64_t inleaves[16];
                        for (int i = 0; i < 4; i++) {
                            std::memcpy(inleaves+(4*i), ind2ptr_leaf(pptr->key.x[i])->key.x+(4*j), 32);
                        }
                        uint32_t d[32];
                        if (bis >= 10) {
                            apg::z64_to_r32_avx2(inleaves, d);
                        } else if (bis >= 9) {
                            apg::z64_to_r32_avx(inleaves, d);
                        } else {
                            apg::z64_to_r32_sse2(inleaves, d);
                        }
                        uint32_t e[32];
                        for (int i = 0; i < 16; i++) {
                            e[i+8] = (d[i+ty] >> tx) << 8;
                        }
                        if (bis >= 10) {
                            apg::r32_centre_to_z64_avx2(e, outleaf.x+(4*j));
                        } else if (bis >= 9) {
                            apg::r32_centre_to_z64_avx(e, outleaf.x+(4*j));
                        } else if (bis >= 7) {
                            apg::r32_centre_to_z64_sse4(e, outleaf.x+(4*j));
                        } else {
                            apg::r32_centre_to_z64_ssse3(e, outleaf.x+(4*j));
                        }
                    }

                    I res = make_leaf(outleaf);
                    memmap->emplace(std::make_pair(hnode.index, hnode.depth), res);
                    return hypernode<I>(res, 0);
                }
            }
        }

        hypernode<I> boolean_recurse(hypernode<I> lnode, hypernode<I> rnode, int operation,
                                        std::map<std::pair<std::pair<I, I>, uint32_t>, I> *memmap) {
            /*
            *   0 = and
            *   1 = or
            *   2 = xor
            *   3 = andn
            */

            auto it = memmap->find(std::make_pair(std::make_pair(lnode.index, rnode.index), lnode.depth));

            if (lnode.index == 0) {
                if (operation == 0 || operation == 3) {
                    return lnode;
                } else {
                    return rnode;
                }
            } else if (rnode.index == 0) {
                if (operation == 0) {
                    return rnode;
                } else {
                    return lnode;
                }
            } else if (it != memmap->end()) {
                return hypernode<I>(it->second, lnode.depth);
            } else if (lnode.depth >= 1){
                kiventry<nicearray<I, 4>, I, lifemeta<I> >* lptr = ind2ptr_nonleaf(lnode.depth, lnode.index);
                kiventry<nicearray<I, 4>, I, lifemeta<I> >* rptr = ind2ptr_nonleaf(rnode.depth, rnode.index);
                hypernode<I> ytl = boolean_recurse(hypernode<I>(lptr->key.x[0], lnode.depth-1), hypernode<I>(rptr->key.x[0], rnode.depth-1),
                                                    operation, memmap);
                hypernode<I> ytr = boolean_recurse(hypernode<I>(lptr->key.x[1], lnode.depth-1), hypernode<I>(rptr->key.x[1], rnode.depth-1),
                                                    operation, memmap);
                hypernode<I> ybl = boolean_recurse(hypernode<I>(lptr->key.x[2], lnode.depth-1), hypernode<I>(rptr->key.x[2], rnode.depth-1),
                                                    operation, memmap);
                hypernode<I> ybr = boolean_recurse(hypernode<I>(lptr->key.x[3], lnode.depth-1), hypernode<I>(rptr->key.x[3], rnode.depth-1),
                                                    operation, memmap);
                nicearray<I, 4> cc = {ytl.index, ytr.index, ybl.index, ybr.index};
                hypernode<I> xcc = make_nonleaf_hn(lnode.depth, cc);
                memmap->emplace(std::make_pair(std::make_pair(lnode.index, rnode.index), lnode.depth), xcc.index);
                return xcc;
            } else {
                kiventry<nicearray<uint64_t, 4*N>, I, lifemeta<I> >* lptr = ind2ptr_leaf(lnode.index);
                kiventry<nicearray<uint64_t, 4*N>, I, lifemeta<I> >* rptr = ind2ptr_leaf(rnode.index);
                nicearray<uint64_t, 4*N> outleaf = {0ull};
                if (operation == 0) {
                    for (int i = 0; i < 4*N; i++) {
                        outleaf.x[i] = lptr->key.x[i] & rptr->key.x[i];
                    }
                } else if (operation == 1) {
                    for (int i = 0; i < 4*N; i++) {
                        outleaf.x[i] = lptr->key.x[i] | rptr->key.x[i];
                    }
                } else if (operation == 2) {
                    for (int i = 0; i < 4*N; i++) {
                        outleaf.x[i] = lptr->key.x[i] ^ rptr->key.x[i];
                    }
                } else {
                    for (int i = 0; i < 4*N; i++) {
                        outleaf.x[i] = lptr->key.x[i] & ~(rptr->key.x[i]);
                    }
                }
                I res = make_leaf(outleaf);
                memmap->emplace(std::make_pair(std::make_pair(lnode.index, rnode.index), 0), res);
                return hypernode<I>(res, 0);
            }
        }

        uint64_t digest_recurse(hypernode<I> hnode, std::map<std::pair<I, uint32_t>, uint64_t> *memmap) {

            auto it = memmap->find(std::make_pair(hnode.index, hnode.depth));
            if (hnode.index == 0) {
                return 0;
            } else if (it != memmap->end()) {
                return it->second;
            } else if (hnode.depth == 0) {
                // This is a leaf node (16-by-16 square); extract its memory location:
                kiventry<nicearray<uint64_t, 4*N>, I, lifemeta<I> >* pptr = ind2ptr_leaf(hnode.index);
                uint64_t h = pptr->key.hash();
                memmap->emplace(std::make_pair(hnode.index, hnode.depth), h);
                return h;
            } else {
                // Extract the pointer to the node:
                kiventry<nicearray<I, 4>, I, lifemeta<I> >* pptr = ind2ptr_nonleaf(hnode.depth, hnode.index);

                // Lazy evaluation:
                uint64_t a = digest_recurse(hypernode<I>(pptr->key.x[0], hnode.depth - 1), memmap) + 314159;
                uint64_t b = digest_recurse(hypernode<I>(pptr->key.x[1], hnode.depth - 1), memmap) + 265358;
                uint64_t c = digest_recurse(hypernode<I>(pptr->key.x[2], hnode.depth - 1), memmap) + 979323;
                uint64_t d = digest_recurse(hypernode<I>(pptr->key.x[3], hnode.depth - 1), memmap) + 846264;

                // Depth-sensitivity:
                a *= (3 + (hnode.depth << 1));
                b *= (5 + (hnode.depth << 2));
                c *= (7 + (hnode.depth << 4));
                d *= (9 + (hnode.depth << 8));

                // Encipher (a, b, c, d):
                a ^= ((b + c) << 11) | ((b + c) >> 53);
                d ^= ((a + b) << 17) | ((a + b) >> 47);
                c ^= ((d + a) << 23) | ((d + a) >> 41);
                b ^= ((c + d) << 59) | ((c + d) >> 5);

                // Extra non-linearity:
                b %= 157079632679489783;

                // Now compute a hash:
                uint64_t h = (65537*a + 257*b + 17*c + 5*d);
                h ^= (h >> 32);
                memmap->emplace(std::make_pair(hnode.index, hnode.depth), h);
                return h;
            }

        }

        hypernode<I> convolve_recurse(hypernode<I> lnode, hypernode<I> rnode, bool exclusive,
                                        std::map<std::pair<std::pair<I, I>, uint32_t>, I> *memmap1,
                                        std::map<std::pair<std::pair<I, I>, uint32_t>, I> *memmap2) {

            auto it = memmap2->find(std::make_pair(std::make_pair(lnode.index, rnode.index), lnode.depth));

            if (lnode.index == 0) {
                return hypernode<I>(0, lnode.depth + 1);
            } else if (rnode.index == 0) {
                return hypernode<I>(0, lnode.depth + 1);
            } else if (it != memmap2->end()) {
                return hypernode<I>(it->second, lnode.depth + 1);
            } else if (lnode.depth >= 1) {
                kiventry<nicearray<I, 4>, I, lifemeta<I> >* lptr = ind2ptr_nonleaf(lnode.depth, lnode.index);
                kiventry<nicearray<I, 4>, I, lifemeta<I> >* rptr = ind2ptr_nonleaf(rnode.depth, rnode.index);

                hypernode<I> pconvs[4][4];
                for (int c = 0; c < 4; c++) {
                    for (int d = 0; d < 4; d++) {
                        pconvs[c][d] = hypernode<I>(0, lnode.depth - 1);
                    }
                }

                // The bool 'exclusive' determines whether the operation is OR or XOR:
                int op = exclusive + 1;

                for (int i = 0; i < 4; i++) {
                    for (int j = 0; j < 4; j++) {
                        hypernode<I> q = convolve_recurse(hypernode<I>(lptr->key.x[i], lnode.depth-1),
                                                          hypernode<I>(rptr->key.x[j], rnode.depth-1),
                                                          exclusive, memmap1, memmap2);
                        if (q.index) {
                            kiventry<nicearray<I, 4>, I, lifemeta<I> >* qptr = ind2ptr_nonleaf(q.depth, q.index);
                            for (int k = 0; k < 4; k++) {
                                hypernode<I> hijk = hypernode<I>(qptr->key.x[k], lnode.depth-1);
                                int a = (i & 1) + (j & 1) + (k & 1);
                                int b = (i & 2) + (j & 2) + (k & 2);
                                int c = ((b & 4) | (a & 2)) >> 1;
                                int d = ((b & 2) | (a & 1));
                                pconvs[c][d] = boolean_recurse(pconvs[c][d], hijk, op, memmap1);
                            }
                        }
                    }
                }

                nicearray<I, 4> xcc;
                for (int c = 0; c < 4; c++) {
                    nicearray<I, 4> cc;
                    for (int d = 0; d < 4; d++) {
                        cc.x[d] = pconvs[c][d].index;
                    }
                    xcc.x[c] = make_nonleaf(lnode.depth, cc);
                }

                hypernode<I> ycc = make_nonleaf_hn(lnode.depth + 1, xcc);
                memmap2->emplace(std::make_pair(std::make_pair(lnode.index, rnode.index), lnode.depth), ycc.index);

                return ycc;
            } else {
                // lnode and rnode are depth-0. As always, base cases are more
                // annoying than the recursion:
                kiventry<nicearray<uint64_t, 4*N>, I, lifemeta<I> >* lptr = ind2ptr_leaf(lnode.index);
                kiventry<nicearray<uint64_t, 4*N>, I, lifemeta<I> >* rptr = ind2ptr_leaf(rnode.index);

                nicearray<uint64_t, 4*N> outleaves[4];

                for (int p = 0; p < N; p++) {
                    uint64_t pconvs[4][4];
                    std::memset(pconvs, 0, 128);
                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < 4; j++) {
                            uint64_t lu = lptr->key.x[4*p + i];
                            uint64_t ru = rptr->key.x[4*p + j];
                            if ((lu != 0) && (ru != 0)) {
                                uint64_t qconvs[4];
                                std::memset(qconvs, 0, 32);
                                uint64_convolve(lu, ru, qconvs, exclusive);
                                for (int k = 0; k < 4; k++) {
                                    int a = (i & 1) + (j & 1) + (k & 1);
                                    int b = (i & 2) + (j & 2) + (k & 2);
                                    int c = ((b & 4) | (a & 2)) >> 1;
                                    int d = ((b & 2) | (a & 1));
                                    if (exclusive) {
                                        pconvs[c][d] ^= qconvs[k];
                                    } else {
                                        pconvs[c][d] |= qconvs[k];
                                    }
                                }
                            }
                        }
                    }
                    for (int c = 0; c < 4; c++) {
                        for (int d = 0; d < 4; d++) {
                            outleaves[c].x[4*p+d] = pconvs[c][d];
                        }
                    }
                }

                nicearray<I, 4> cc;
                for (int c = 0; c < 4; c++) {
                    cc.x[c] = make_leaf(outleaves[c]);
                }

                hypernode<I> ycc = make_nonleaf_hn(lnode.depth + 1, cc);
                memmap2->emplace(std::make_pair(std::make_pair(lnode.index, rnode.index), lnode.depth), ycc.index);

                return ycc;
            }

        }

        hypernode<I> solid(uint32_t depth) {
            nicearray<uint64_t, 4*N> outleaf;
            std::memset(outleaf.x, 255, 32*N);
            hypernode<I> hnode(make_leaf(outleaf), 0);
            while (hnode.depth < depth) {
                nicearray<I, 4> cc = {hnode.index, hnode.index, hnode.index, hnode.index};
                hnode = make_nonleaf_hn(hnode.depth + 1, cc);
            }
            return hnode;
        }

        hypernode<I> invstring_recurse(std::string &str, uint64_t &loc) {
            char ch = str[loc++];
            if ((ch >= 'F') && (ch <= 'Z')) {
                hypernode<I> a = invstring_recurse(str, loc);
                hypernode<I> b = invstring_recurse(str, loc);
                hypernode<I> c = invstring_recurse(str, loc);
                hypernode<I> d = invstring_recurse(str, loc);
                nicearray<I, 4> cc = {a.index, b.index, c.index, d.index};
                hypernode<I> hnode = make_nonleaf_hn((uint32_t) (ch - 'F'), cc);
                return hnode;
            } else if (ch == '0') {
                return hypernode<I>(0, 1);
            } else {
                int l = 0;
                if ((ch >= '0') && (ch <= '9')) { l = ch - '0'; }
                if ((ch >= 'a') && (ch <= 'z')) { l = (ch - 'a') + 10; }
                if ((ch >= 'A') && (ch <= 'E')) { l = (ch - 'A') + 36; }
                std::string s = str.substr(loc, l);
                loc += l;
                uint32_t a[8] = {0};
                base85decode(a, s, 8);
                nicearray<uint64_t, 4*N> ln = {0ull};
                std::memcpy(ln.x, a, 32);
                return hypernode<I>(make_leaf(ln), 0);
            }
        }

        std::string string_recurse(hypernode<I> hnode) {
            /*
            * Compute a flat representation of the first layer of a hypernode.
            */
            std::string str = "";
            if (hnode.index == 0) {
                str = "0";
            } else if (hnode.depth == 0) {
                uint32_t a[8] = {0};
                std::memcpy(a, ind2ptr_leaf(hnode.index)->key.x, 32);
                for (int i = 0; i < 8; i++) { str += base85encode(a[i]); }
                str = str.substr(0, 1 + str.find_last_not_of('.'));
                str = "0123456789abcdefghijklmnopqrstuvwxyzABCDE"[str.length()] + str;
            } else {
                str = "FGHIJKLMNOPQRSTUVWXYZ"[hnode.depth];
                for (int i = 0; i < 4; i++) { str += string_recurse(getchild(hnode, i)); }
            }
            return str;
        }

        std::string _string32(hypernode<I> hnode) { return string_recurse(hnode); }

        hypernode<I> _string32(std::string s) {
            uint64_t loc = 0;
            hypernode<I> hnode = invstring_recurse(s, loc);
            if ((s.length() > loc) && (s[loc] != ' ')) {std::cerr << "Very bad" << std::endl; }
            return hnode;
        }

        void bitworld_recurse(hypernode<I> hnode, bitworld* bw, uint32_t layer, int32_t x, int32_t y) {
            if (hnode.index == 0) {
                return;
            } else if (hnode.depth == 0) {
                kiventry<nicearray<uint64_t, 4*N>, I, lifemeta<I> >* pptr = ind2ptr_leaf(hnode.index);
                bw->world.emplace(std::pair<int32_t, int32_t>(x, y), pptr->key.x[4*layer]);
                bw->world.emplace(std::pair<int32_t, int32_t>(x+1, y), pptr->key.x[4*layer+1]);
                bw->world.emplace(std::pair<int32_t, int32_t>(x, y+1), pptr->key.x[4*layer+2]);
                bw->world.emplace(std::pair<int32_t, int32_t>(x+1, y+1), pptr->key.x[4*layer+3]);
            } else {
                bitworld_recurse(getchild(hnode, 0), bw, layer, x, y);
                bitworld_recurse(getchild(hnode, 1), bw, layer, x + (1 << hnode.depth), y);
                bitworld_recurse(getchild(hnode, 2), bw, layer, x, y + (1 << hnode.depth));
                bitworld_recurse(getchild(hnode, 3), bw, layer, x + (1 << hnode.depth), y + (1 << hnode.depth));
            }
        }

        hypernode<I> demorton_recurse(std::map<uint64_t, uint64_t>::iterator &it,
                                      std::map<uint64_t, uint64_t>::iterator &pe,
                                      uint64_t lm, uint64_t upto, uint32_t depth) {

            // If we've reached the end of the map:
            if (it == pe) { return hypernode<I>(0, depth); }
            uint64_t location = it->first;
            if ((upto == 0) || (location < upto)) {
                if (depth == 0) {
                    nicearray<uint64_t, 4*N> outleaf;
                    std::memset(outleaf.x, 0, 32*N);
                    while ((upto == 0) || (location < upto)) {
                        uint64_t value = it->second;
                        for (int i = 0; i < N; i++) {
                            if ((lm >> i) & 1) {
                                outleaf.x[4*i + (location & 3)] = value;
                            }
                        }
                        ++it;
                        if (it == pe) { break; }
                        location = it->first;
                    }
                    return hypernode<I>(make_leaf(outleaf), 0);
                } else {
                    nicearray<I, 4> cc;
                    cc.x[0] = demorton_recurse(it, pe, lm, upto - (3ull << (2*depth)), depth-1).index;
                    cc.x[1] = demorton_recurse(it, pe, lm, upto - (2ull << (2*depth)), depth-1).index;
                    cc.x[2] = demorton_recurse(it, pe, lm, upto - (1ull << (2*depth)), depth-1).index;
                    cc.x[3] = demorton_recurse(it, pe, lm, upto, depth-1).index;
                    return make_nonleaf_hn(depth, cc);
                }
            } else {
                return hypernode<I>(0, depth);
            }
        }

        lifetree(uint64_t maxmem) {
            // maxmem is specified in MiB, so we left-shift by 20:
            this->gc_threshold = maxmem << 20;
        }
    };

}
