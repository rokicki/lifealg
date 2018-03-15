#include "lifealgo.h"
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <algorithm>
using namespace std ;
typedef unsigned long long ull ;
typedef long long ll ;
const ll END = 0x7ffffffffffffff0LL ;
class list8x8algo : public lifealgo {
public:
   virtual void init(int w, int h) ;
   virtual void setcell(int x, int y) ;
   virtual int getpopulation() ;
   virtual int nextstep(int, int) ;
   virtual void swap() ;
   void createit() ;
   void flushit(vector<pair<pair<ll, ll>, ull> > &trow) ;
   int created ;
   vector<pair<int, int> > pts ;
   vector<ll> a0, a1 ;
} ;
static class list8x8algofactory : public lifealgofactory {
public:
   list8x8algofactory() ;
   virtual lifealgo *createInstance() {
      return new list8x8algo() ;
   }
} factory ;
list8x8algofactory::list8x8algofactory() {
   registerAlgo("list8x8", &factory) ;
}
void list8x8algo::init(int w_, int h_) {
}
void list8x8algo::setcell(int x, int y) {
   created = 0 ;
   pts.push_back(make_pair(y, x)) ;
}
static int popcount64(ull v) {
   return __builtin_popcount(v) + __builtin_popcount(v >> 32) ;
}
int list8x8algo::getpopulation() {
   if (!created)
      return pts.size() ;
   int r = 0 ;
   int at = 0 ;
   while (a0[at] != END) { // for each row
      at++ ;
      while (a0[at] != END) { // for each point 
         at++ ;
         r += popcount64(a0[at++]) ;
      }
      at++ ;
   }
   return r ;
}
void list8x8algo::swap() { }
void list8x8algo::flushit(vector<pair<pair<ll, ll>, ull> > &trow) {
   if (trow.size() == 0)
      return ;
   sort(trow.begin(), trow.end()) ;
   a0.push_back(trow[0].first.first << 3) ;
   for (int i=0; i<trow.size(); ) {
      ull bits = trow[i].second ;
      int j = 1 ;
      while (i+j < trow.size() && trow[i].first == trow[i+j].first) {
         bits |= trow[i+j].second ;
         j++ ;
      }
      a0.push_back(trow[i].first.second << 3) ;
      a0.push_back(bits) ;
      i += j ;
   }
   a0.push_back(END) ;
   trow.clear() ;
}
void list8x8algo::createit() {
   if (!created) {
      sort(pts.begin(), pts.end()) ;
      ll cury = END ;
      vector<pair<pair<ll, ll>, ull> > trow ;
      for (ll i=0; i<pts.size(); i++) {
         ll yy = pts[i].first >> 3 ;
         if (yy != cury) {
            flushit(trow) ;
            cury = yy ;
         }
         trow.push_back(make_pair(make_pair(yy, pts[i].second >> 3),
            1LL << ((pts[i].second & 7) + 8 * (pts[i].first & 7)))) ;
      }
      flushit(trow) ;
      if (cury != END)
         a0.push_back(END) ;
      created = 1 ;
   }
}
static inline void add2(ull a, ull b,
                        ull &c0, ull &c1) {
   c0 = a ^ b ;
   c1 = a & b ;
}
static inline void add3(ull a, ull b, ull c,
                        ull &c0, ull &c1) {
   ull t0, t1, t2 ;
   add2(a, b, t0, t1) ;
   add2(t0, c, c0, t2) ;
   c1 = t2 | t1 ;
}
/*
 *   Calculate the 8x8 universe one down and right from the 16x16 universe
 *   in the four provided cells; each is an 8x8 universe.  We use bit tricks.
 */
static ull calc(ull nw, ull ne, ull sw, ull se) {
   ull n0, n1, s0, s1, a0, a1, a2, c1, b1, b2 ;
   add3(nw,
        ((nw >> 1) & 0x7f7f7f7f7f7f7f7fLL) + ((ne << 7) & 0x8080808080808080LL),
        ((nw >> 2) & 0x3f3f3f3f3f3f3f3fLL) + ((ne << 6) & 0xc0c0c0c0c0c0c0c0LL),
        n0, n1) ;
   add3(sw,
        ((sw >> 1) & 0x7f7f7f7f7f7f7f7fLL) + ((se << 7) & 0x8080808080808080LL),
        ((sw >> 2) & 0x3f3f3f3f3f3f3f3fLL) + ((se << 6) & 0xc0c0c0c0c0c0c0c0LL),
        s0, s1) ;
   ull n10 = (n0 >> 8) + (s0 << 56) ;
   ull n11 = (n1 >> 8) + (s1 << 56) ;
   ull n20 = (n0 >> 16) + (s0 << 48) ;
   ull n21 = (n1 >> 16) + (s1 << 48) ;
   add3(n0, n10, n20, a0, c1) ;
   add3(n1, n11, n21, b1, a2) ;
   a1 = b1 ^ c1 ;
   a2 ^= b1 & c1 ;
   ull curgen = ((nw >> 9) & 0x007f7f7f7f7f7f7fLL) +
                ((ne >> 1) & 0x0080808080808080LL) +
                ((sw << 55) & 0x7f00000000000000LL) +
                 (se << 63) ;
   return (~(a0 ^ a1)) & (a1 ^ a2) & (curgen | a1) ;
}
int list8x8algo::nextstep(int id, int nid) {
   int r = 0 ;
   if (nid != 1)
      error("! multithreading not yet supported") ;
   if (!created)
      createit() ;
   a1.clear() ;
   ll r0=0, r1=0 ;
   ll wy = END ;
   while (a0[r0] != END) {
      ll y = min(a0[r0], a0[r1]-8) ;
      ll p0 = r0+1 ;
      ll p1 = r1 ;
      if (a0[p1] != END)
         p1++ ;
      ll x = -END ;
      while (1) {
         ll xt = END ;
         if (a0[r0] == y)
            xt = a0[p0] ;
         if (a0[r1] == y+8)
            xt = min(xt, a0[p1]) ;
         if (xt == END) {
            if (a0[r0] == y)
               r0 = p0 + 1 ;
            if (a0[r1] == y+8)
               r1 = p1 + 1 ;
            break ;
         }
         x = max(x, xt-8) ;
         ull nw=0, ne=0, sw=0, se=0 ;
         if (a0[r0] == y) {
            if (a0[p0] == x) {
               nw = a0[p0+1] ;
               p0 += 2 ;
            }
            if (a0[p0] == x+8)
               ne = a0[p0+1] ;
         }
         if (a0[r1] == y+8) {
            if (a0[p1] == x) {
               sw = a0[p1+1] ;
               p1 += 2 ;
            }
            if (a0[p1] == x+8)
               se = a0[p1+1] ;
         }
         ull nb = calc(nw, ne, sw, se) ;
         if (nb != 0) {
            if (wy != y+1) {
               if (wy != END)
                  a1.push_back(END) ;
               wy = y+1 ;
               a1.push_back(y+1) ;
            }
            a1.push_back(x+1) ;
            a1.push_back(nb) ;
            r += popcount64(nb) ;
         }
         x += 8 ;
      }
   }
   a1.push_back(END) ;
   if (wy != END)
      a1.push_back(END) ;
   ::swap(a0, a1) ;
   return r ;
}
