#include "lifealgo.h"
#include "util.h"
#include <algorithm>
#include <cstdlib>
using namespace std ;
typedef unsigned long long ull ;
class bitparalgo : public lifealgo {
public:
   virtual void init(int w, int h) ;
   virtual void setcell(int x, int y) ;
   virtual int getpopulation() ;
   virtual int nextstep(int, int, int) ;
   virtual void swap() ;
   int w, h, wordwidth ;
   long long wh ;
   ull *u0, *u1 ;
} ;
static class bitparalgofactory : public lifealgofactory {
public:
   bitparalgofactory() ;
   virtual lifealgo *createInstance() {
      return new bitparalgo() ;
   }
} factory ;
bitparalgofactory::bitparalgofactory() {
   registerAlgo("bitpar", &factory) ;
}
void bitparalgo::init(int w_, int h_) {
   w = w_ ;
   h = h_ ;
   wordwidth = (w + 63) >> 6 ;
   wh = wordwidth * h ;
   u0 = (ull *)calloc(wordwidth*sizeof(ull), h+1) ;
   u1 = (ull *)calloc(wordwidth*sizeof(ull), h+1) ;
}
void bitparalgo::setcell(int x, int y) {
   u0[(x>>6)*h+y] |= 1LL << (x & 63) ;
}
int bitparalgo::getpopulation() {
   int r = 0 ;
   for (int i=0; i<wh; i++)
      r += __builtin_popcountll(u0[i]) ;
   return r ;
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
void bitparalgo::swap() { ::swap(u0, u1) ; }
int bitparalgo::nextstep(int i, int n, int needpop) {
   int r = 0 ;
   int loi = i * wordwidth / n ;
   int hii = (i + 1) * wordwidth / n ;
   for (int i=loi; i<hii; i++) {
      ull w00 = 0 ;
      ull w01 = 0 ;
      ull *col = u0 + i * h + 1 ;
      ull *pcol = u0 + (i-1) * h + 1 ;
      ull *ncol = u0 + (i+1) * h + 1 ;
      ull *wcol = u1 + i * h + 1 ;
      ull w1 = *col ;
      ull w1l = w1 << 1 ;
      ull w1r = w1 >> 1 ;
      if (i > 0)
         w1l += *pcol >> (wordwidth-1) ;
      if (i+1 < wordwidth)
         w1r += *ncol << (wordwidth-1) ;
      ull w10, w11 ;
      add3(w1, w1l, w1r, w10, w11) ;
      for (int j=1; j+1<h; j += 2, col += 2, pcol += 2, ncol += 2, wcol += 2) {
         ull w2 = col[1] ;
         ull w3 = 0 ;
         w3 = col[2] ;
         ull w2l = w2 << 1 ;
         ull w3l = w3 << 1 ;
         ull w2r = w2 >> 1 ;
         ull w3r = w3 >> 1 ;
         if (i > 0) {
            w2l |= pcol[1] >> 63 ;
            w3l |= pcol[2] >> 63 ;
         }
         if (i+1 < wordwidth) {
            w2r |= ncol[1] << 63 ;
            w3r |= ncol[2] << 63 ;
         }
         ull w20, w21, w30, w31, a0, a1, a2, b0, b1, b2, ng1, ng2 ;
         add3(w2, w2l, w2r, w20, w21) ;
         add2(w10, w20, b0, a1) ;
         add3(w11, w21, a1, b1, b2) ;
         add2(b0, w00, a0, a1) ;
         add3(b1, w01, a1, a1, a2) ;
         a2 ^= b2 ;
         ng1 = (a0 ^ a2) & (a1 ^ a2) & (w1 | a1) ;
         wcol[0] = ng1 ;
         add3(w3, w3l, w3r, w30, w31) ;
         add2(b0, w30, a0, a1) ;
         add3(b1, w31, a1, a1, a2) ;
         a2 ^= b2 ;
         ng2 = (a0 ^ a2) & (a1 ^ a2) & (w2 | a1) ;
         wcol[1] = ng2 ;
         if (needpop)
            r += __builtin_popcountll(ng1) + __builtin_popcountll(ng2) ;
         w00 = w20 ;
         w01 = w21 ;
         w10 = w30 ;
         w11 = w31 ;
         w1 = w3 ;
      }
   }
   return r ;
}
