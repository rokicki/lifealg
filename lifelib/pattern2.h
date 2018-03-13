#pragma once
#include "lifetree.h"

namespace apg {

    class pattern {
        /*
        * Patterns with dynamic garbage collection.
        * A pattern can be instantiated either from a macrocell file:
        *
        * pattern x(&lt, "filename.mc");
        *
        * or from an RLE literal:
        *
        * pattern glider(&lt, "3o$o$bo!", "b3s23");
        */

        hypernode<uint32_t> hnode;
        uint64_t ihandle;
        std::string rulestring;
        lifetree_abstract<uint32_t>* lab;

        public:

        uint64_t minp;
        uint64_t dt;
        int64_t dx;
        int64_t dy;

        lifetree_abstract<uint32_t>* getlab() const { return lab; }
        hypernode<uint32_t> gethnode() const { return hnode; }
        std::string getrule() const { return rulestring; }
        void setrule(std::string rule) { rulestring = rule; }

        bitworld flatlayer(uint32_t layer) {
            return lab->flatlayer(hnode, layer);
        }

        std::string _string32() { return lab->_string32(hnode); }

        // We include some constructors:

        pattern(lifetree_abstract<uint32_t> *lab, hypernode<uint32_t> hnode, std::string rulestring,
                int64_t dx, int64_t dy, uint64_t dt, uint64_t minp) {
            this->lab = lab;
            this->ihandle = lab->newihandle(hnode);
            this->hnode = hnode;
            this->rulestring = rulestring;
            this->dt = dt; this->minp = minp; this->dx = dx; this->dy = dy;
        }

        pattern(lifetree_abstract<uint32_t> *lab, hypernode<uint32_t> hnode, std::string rulestring) {
            this->lab = lab;
            this->ihandle = lab->newihandle(hnode);
            this->hnode = hnode;
            this->rulestring = rulestring;
            this->dt = 0; this->minp = 0; this->dx = 0; this->dy = 0;
        }

        pattern(lifetree_abstract<uint32_t> *lab, std::vector<bitworld> planes, std::string rulestring) {
            this->lab = lab;
            this->hnode = lab->fromplanes(planes);
            this->ihandle = lab->newihandle(this->hnode);
            this->rulestring = rulestring;
            this->dt = 0; this->minp = 0; this->dx = 0; this->dy = 0;
        }

        pattern(lifetree_abstract<uint32_t> *lab, std::string rle, std::string rulestring) {
            this->lab = lab;
            this->hnode = lab->fromrle(rle);
            this->ihandle = lab->newihandle(this->hnode);
            this->rulestring = rulestring;
            this->dt = 0; this->minp = 0; this->dx = 0; this->dy = 0;
        }

        pattern(lifetree_abstract<uint32_t> *lab, std::string filename) {
            std::string rule = "b3s23";
            hypernode<uint32_t> loaded = lab->load_macrocell(filename, rule);
            this->lab = lab;
            this->ihandle = lab->newihandle(loaded);
            this->hnode = loaded;
            this->rulestring = rule;
            this->dt = 0; this->minp = 0; this->dx = 0; this->dy = 0;
        }

        // Copy a pattern from a different lifetree:

        hypernode<uint32_t> coerce(const pattern &other) const {
            lifetree_abstract<uint32_t>* lab2 = other.getlab();
            hypernode<uint32_t> hnode2 = other.gethnode();
            if (lab2 == lab) {
                return hnode2;
            } else {
                // std::cerr << "WARNING: Non-trivial coercion" << std::endl;
                return lab->copy_recurse(other.gethnode(), other.getlab());
            }
        }

        // The pattern class manages resources (the associated lifetree keeps
        // a handle so that the pattern is saved from garbage-collection);
        // these need to be released when appropriate.

        pattern(const pattern &p) {
            lab = p.getlab();
            hnode = p.gethnode();
            rulestring = p.getrule();
            dx = p.dx; dy = p.dy; dt = p.dt; minp = p.minp;
            ihandle = lab->newihandle(hnode);
        }

        pattern& operator=(const pattern &p) {
            hnode = coerce(p);
            lab->sethandle(ihandle, hnode);
            dx = p.dx; dy = p.dy; dt = p.dt; minp = p.minp;
            rulestring = p.getrule();
            return *this;
        }

        ~pattern() {
            lab->delhandle(ihandle);
            lab->threshold_gc();
        }

        // End of resource-management code.

        // Pattern advancing:

        pattern advance(std::string rule, uint64_t numgens) {
            if (rule == rulestring) {
                return pattern(lab, lab->advance(hnode, rule, numgens), rule, dx, dy, dt, minp);
            } else {
                return pattern(lab, lab->advance(hnode, rule, numgens), rule);
            }
        }

        pattern advance(uint64_t numgens) {
            return advance(rulestring, numgens);
        }

        pattern operator[](std::string rule) {
            return pattern(lab, hnode, rule);
        }

        // Shifts and transformations:

        pattern shift(int64_t x, int64_t y) {
            return pattern(lab, lab->shift_universe(hnode, x, y), rulestring, dx, dy, dt, minp);
        }

        pattern getchild(uint32_t x) const {
            return pattern(lab, lab->getchild(hnode, x), rulestring);
        }

        pattern transform(std::string tfm, int64_t x, int64_t y) {
            uint8_t perm = 228;
            if ((tfm == "flip") || (tfm == "rot180")) {
                perm = 27;
            } else if (tfm == "flip_x") {
                perm = 177;
            } else if (tfm == "flip_y") {
                perm = 78;
            } else if ((tfm == "swap_xy") || (tfm == "transpose")) {
                perm = 216;
            } else if (tfm == "swap_xy_flip") {
                perm = 39;
            } else if ((tfm == "rcw") || (tfm == "rot270")) {
                perm = 114;
            } else if ((tfm == "rccw") || (tfm == "rot90")) {
                perm = 141;
            }
            return pattern(lab, lab->transform_and_shift(hnode, perm, x, y), rulestring);
        }

        pattern transpose() { return transform("transpose", 0, 0); }

        pattern operator()(int64_t x, int64_t y) {
            return shift(x, y);
        }

        pattern operator()(std::string tfm, int64_t x, int64_t y) {
            return transform(tfm, x, y);
        }

        // Rectangle acquisition/selection/removal:

        pattern subrect(int64_t x, int64_t y, uint64_t width, uint64_t height) {
            hypernode<uint32_t> rect = lab->rectangle(x, y, width, height);
            return pattern(lab, lab->boolean_universe(hnode, rect, 0), rulestring);
        }

        pattern subrect(int64_t *bbox) {
            return subrect(bbox[0], bbox[1], bbox[2], bbox[3]);
        }

        pattern rmrect(int64_t x, int64_t y, uint64_t width, uint64_t height) {
            hypernode<uint32_t> rect = lab->rectangle(x, y, width, height);
            return pattern(lab, lab->boolean_universe(hnode, rect, 3), rulestring);
        }

        pattern rmrect(int64_t *bbox) {
            return rmrect(bbox[0], bbox[1], bbox[2], bbox[3]);
        }

        bool getrect(int64_t *bbox) {
            return lab->getbbox(hnode, bbox);
        }

        void display(int64_t *bbox) {
            int dim = 128;
            if (hnode.depth == 2) { dim = 64; }
            if (hnode.depth == 1) { dim = 32; }
            if (hnode.depth == 0) { dim = 16; }

            int startx = (bbox == 0) ? 0 : bbox[0] + (8 << hnode.depth);
            int starty = (bbox == 0) ? 0 : bbox[1] + (8 << hnode.depth);
            int endx = (bbox == 0) ? dim : bbox[2];
            int endy = (bbox == 0) ? dim : bbox[3];

            for (int y = starty; y < starty + endy; y++) {
                for (int x = startx; x < startx + endx; x++) {
                    uint64_t c = lab->getcell_recurse(hnode, x, y);
                    if (c > 1) {std::cout << "\033[3" << ((c >> 1) % 7) + 1 << ";1m"; }
                    if (c > 0) {std::cout << "_*.o"[c & 3]; } else { std::cout << " "; }
                    if (c > 1) {std::cout << "\033[0m"; }
                }
                std::cout << std::endl;
            }
        }

        void display() { display(0); }

        // Set operations (mutable):

        void changehnode(hypernode<uint32_t> hn) {
            hnode = hn;
            lab->sethandle(ihandle, hnode);
            this->dt = 0; this->minp = 0; this->dx = 0; this->dy = 0;
        }

        pattern& operator&=(const pattern &other) {
            changehnode(lab->boolean_universe(hnode, coerce(other), 0));
            return *this;
        }

        pattern& operator|=(const pattern &other) {
            changehnode(lab->boolean_universe(hnode, coerce(other), 1));
            return *this;
        }

        pattern& operator^=(const pattern &other) {
            changehnode(lab->boolean_universe(hnode, coerce(other), 2));
            return *this;
        }

        pattern& operator-=(const pattern &other) {
            changehnode(lab->boolean_universe(hnode, coerce(other), 3));
            return *this;
        }

        pattern& operator+=(const pattern &other) {
            changehnode(lab->boolean_universe(hnode, coerce(other), 1));
            return *this;
        }

        // Set operations (immutable):

        pattern conjunction(const pattern &other) {
            return pattern(lab, lab->boolean_universe(hnode, coerce(other), 0), rulestring);
        }

        pattern disjunction(const pattern &other) {
            return pattern(lab, lab->boolean_universe(hnode, coerce(other), 1), rulestring);
        }

        pattern exclusive_disjunction(const pattern &other) {
            return pattern(lab, lab->boolean_universe(hnode, coerce(other), 2), rulestring);
        }

        pattern minus(const pattern &other) {
            return pattern(lab, lab->boolean_universe(hnode, coerce(other), 3), rulestring);
        }

        // Equality testing:

        bool operator==(const pattern &other) const {
            hypernode<uint32_t> l = lab->pyramid_down(hnode);
            hypernode<uint32_t> r = lab->pyramid_down(coerce(other));
            return ((l.depth == r.depth) && (l.index == r.index));
        }

        bool operator!=(const pattern &other) const {
            return !(*this == other);
        }

        bool empty() const { return (this->hnode.index == 0); }

        bool nonempty() const { return (this->hnode.index != 0); }

        // Population counts:

        uint32_t popcount(uint32_t modprime, uint64_t layermask) {
            return lab->getpop_recurse(hnode, modprime, layermask);
        }

        uint32_t popcount(uint32_t modprime) {
            return this->popcount(modprime, -1);
        }

        // Pattern matching:

        pattern convolve(const pattern &other) {
            return pattern(lab, lab->convolve_universe(hnode, coerce(other), false), rulestring);
        }

        pattern xor_convolve(const pattern &other) {
            return pattern(lab, lab->convolve_universe(hnode, coerce(other), true), rulestring);
        }

        pattern match(const pattern &c1, const pattern &c0) {
            return pattern(lab, lab->pattern_match(hnode, coerce(c0), coerce(c1)), rulestring);
        }

        pattern match(const pattern &c1) {
            return pattern(lab, lab->pattern_match(hnode, hypernode<uint32_t>(0, 1), coerce(c1)), rulestring);
        }

        // Periodicity detection:

        uint64_t digest() {
            return lab->digest_universe(hnode);
        }

        void pdetect(uint64_t gmax) {
            /*
            * Detect periodicity. This does not necessarily find the minimum
            * period, but instead can return a multiple of the actual period.
            * For the latter, use the slightly slower ascertain_period().
            */

            if (dt != 0) { return; }

            int64_t bbox[4] = {0};
            int64_t bbox_orig[4] = {0};
            getrect(bbox_orig);

            pattern x = advance(8);
            if ((hnode.depth == x.gethnode().depth) && (hnode.index == x.gethnode().index)) {
                this->minp = (rulestring[1] == '0') ? 2 : 1;
                this->dt = 8;
                this->dx = 0;
                this->dy = 0;
                return;
            }

            uint64_t g = 8;
            uint64_t i = 8;

            std::map<std::pair<uint64_t, std::pair<int64_t, int64_t> >, uint64_t> gmap;

            while (g < gmax) {

                x.getrect(bbox);
                uint64_t h = x.shift(bbox_orig[0] - bbox[0], bbox_orig[1] - bbox[1]).digest();
                std::pair<int64_t, int64_t> p(bbox[2], bbox[3]);
                std::pair<uint64_t, std::pair<int64_t, int64_t> > q(h, p);

                uint64_t g2 = gmap[q];
                if (g2) {
                    uint64_t delta = g - g2;
                    // std::cout << "Possible super-period of " << delta << std::endl;
                    pattern y = advance(delta);
                    y.getrect(bbox);
                    hypernode<uint32_t> hnode2 = y.shift(bbox_orig[0] - bbox[0], bbox_orig[1] - bbox[1]).gethnode();
                    if (hnode.depth == hnode2.depth && hnode.index == hnode2.index) {
                        // std::cout << "Yes!" << std::endl;
                        this->minp = (rulestring[1] == '0') ? 2 : 1;
                        this->dt = delta;
                        this->dx = bbox[0] - bbox_orig[0];
                        this->dy = bbox[1] - bbox_orig[1];
                        return;
                    } else {
                        // std::cout << "No!" << std::endl;
                    }
                } else {
                    gmap[q] = g;
                }

                x = x.advance(i);
                g += i;

                if ((i << 10) == 0) { return; }

                // Increase the step size:
                if (g % (i << 10) == 0) { i *= 2; }

            }
        }

        void pdetect() { pdetect(-1); }

        uint64_t ascertain_period() {
            /*
            * Determine the period of an oscillator or spaceship.
            */

            pdetect();
            if (dt == 0) { return 0; }
            if (dt == minp) { return dt; }

            if ((dx == 0) && (dy == 0)) {
                for (uint64_t j = 1; j <= 8; j++) {
                    uint64_t i = j * minp;
                    if ((dt % i == 0) && ((*this) == advance(i))) {
                        dt = i;
                        minp = i;
                        return i;
                    }
                }
            }

            int64_t bbox[4] = {0};
            int64_t bbox_orig[4] = {0};
            getrect(bbox_orig);

            std::map<uint64_t, uint64_t> factors = factorise64(dt);

            // We don't yet know the period exactly, but we have the bounds
            // minp | period | dt, where | indicates 'is a divisor of'.
            while (minp != dt) {
                std::map<uint64_t, uint64_t>::iterator it;
                for (it = factors.begin(); it != factors.end(); ++it ) {
                    if (it->second != 0) {
                        // We iterate over all maximal prime-power factors p^k:
                        uint64_t peterkay = modexp_u64(it->first, it->second, -1);
                        if (minp % peterkay) {
                            // Determine whether p^k divides the actual period:
                            uint64_t noverp = dt / it->first;
                            pattern x = advance(noverp);
                            x.getrect(bbox);
                            x = x.shift(bbox_orig[0] - bbox[0], bbox_orig[1] - bbox[1]);
                            if (*this == x) {
                                // The period can be divided by p:
                                dt = noverp;
                                this->dx = bbox[0] - bbox_orig[0];
                                this->dy = bbox[1] - bbox_orig[1];
                                it->second -= 1;
                                break;
                            } else {
                                // We can increase our lower-bound on p:
                                minp = euclid_lcm(minp, peterkay);
                            }
                        }
                    }
                }
            }
            return dt;
        }

        std::string phase_wechsler() {
            /*
            * Returns the extended Wechsler format for the current phase:
            */
            int64_t bbox[4] = {0};
            if (getrect(bbox)) {

                if ((bbox[2] + 2) * (bbox[3] + 2) > 5000) { return "#"; }

                uint64_t n_layers = lab->countlayers();
                std::vector<bitworld> bwv;
                for (uint64_t i = 0; i < n_layers; i++) {
                    bwv.push_back(flatlayer(i));
                }
                while ((bwv.size() >= 2) && (bwv.back().population() == 0)) {
                    // Remove redundant '_0' suffices:
                    bwv.pop_back();
                }
                std::string s = wechslerise(bwv, bbox);
                return ((s.length() > 1280) ? "#" : s);
            } else {
                return "0";
            }
        }

        std::string apgcode() {
            /*
            * Returns the apgcode of a still-life, oscillator or spaceship:
            */

            std::string rep = "#";
            uint64_t p = ascertain_period();
            pattern x = advance(0);

            for (uint64_t t = 0; t < p; t++) {
                if (t != 0) { x = x.advance((rulestring[1] == '0') ? 2 : 1); }
                // rep = comprep(rep, x.flatlayer(0).wechsler());
                rep = comprep(rep, x.phase_wechsler());
            }

            std::ostringstream ss;
            ss << ((rep == "#") ? "ov_" : "x");

            if ((p == 1) && (dx == 0) && (dy == 0)) {
                ss << "s" << x.flatlayer(0).population();
            } else {
                if (dx == 0 && dy == 0) {
                    ss << "p" << p;
                } else {
                    ss << "q" << p;
                }
            }

            if (rep != "#") { ss << "_" << rep; }

            return ss.str();
        }

        pattern advance2(int64_t numgens) {
            /*
            * Advance a pattern by a number of generations. If the pattern
            * is periodic (i.e. an oscillator or spaceship), a negative value
            * is admissible; otherwise, numgens must be non-negative.
            */
            if (numgens < 0) { pdetect(); }
            if (dt != 0) {
                int64_t ng = numgens % ((int64_t) dt);
                while (ng < 0) { ng += dt; } // because signed modulo is evil
                int64_t xshift = ((numgens - ng) / ((int64_t) dt)) * dx;
                int64_t yshift = ((numgens - ng) / ((int64_t) dt)) * dy;
                return advance(ng).shift(xshift, yshift);
            } else {
                return advance(numgens);
            }
        }

        pattern stream(std::vector<int64_t> &spacings) {
            /*
            * Construct a spaceship stream where delays between successive
            * gliders are given by the elements of 'spacings'.
            *
            * Example usage: glider.stream({0,109,255,94,255,255,92,256,135,0})
            */
            int64_t cum = 0;
            pattern x = (*this); x -= x;
            for (uint64_t i = 0; i < spacings.size() - 1; i++) {
                cum -= spacings[i];
                x += advance2(cum);
            }
            return x;
        }

        pattern operator[](int64_t numgens) { return advance2(numgens); }

        pattern tensor(const pattern &other, const pattern &other2) {
            lifetree_abstract<uint32_t>* lab2 = other.getlab();
            hypernode<uint32_t> hnode2 = other.gethnode();
            uint32_t delta = hnode2.depth + 4;
            std::vector<uint32_t> v;
            v.push_back(0);
            v.push_back(hnode2.index);
            v.push_back(other2.gethnode().index);
            v.push_back(hnode2.index);
            hypernode<uint32_t> hnode3 = lab2->tensor_recurse(hnode, lab, delta, v);
            return pattern(lab2, hnode3, other.getrule());
        }

        pattern metafy(const pattern &other, const pattern &other2) {
            uint64_t trans = 8 << other.gethnode().depth;
            pattern x = tensor(other.getchild(0), other2.getchild(0));
            x += tensor(other.getchild(1), other2.getchild(1)).shift(trans, 0);
            x += tensor(other.getchild(2), other2.getchild(2)).shift(0, trans);
            x += tensor(other.getchild(3), other2.getchild(3)).shift(trans, trans);
            return x;
        }

        pattern centre() {
            int64_t bbox[4];
            getrect(bbox);
            return shift(-bbox[0]-bbox[2]/2, -bbox[1]-bbox[3]/2);
        }

        void write_macrocell(std::ostream &outstream) {
            lab->write_macrocell(outstream, hnode, rulestring);
        }

    };

    pattern operator+(pattern lhs, const pattern &rhs) { return lhs.disjunction(rhs); }
    pattern operator-(pattern lhs, const pattern &rhs) { return lhs.minus(rhs); }
    pattern operator&(pattern lhs, const pattern &rhs) { return lhs.conjunction(rhs); }
    pattern operator|(pattern lhs, const pattern &rhs) { return lhs.disjunction(rhs); }
    pattern operator^(pattern lhs, const pattern &rhs) { return lhs.exclusive_disjunction(rhs); }


}

