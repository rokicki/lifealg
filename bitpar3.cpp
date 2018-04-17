#include "lifealgo.h"
#include "util.h"
#include <algorithm>
#include <cstdlib>
using namespace std ;
typedef unsigned long long ull ;
class bitpar3algo : public lifealgo {
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
static class bitpar3algofactory : public lifealgofactory {
public:
   bitpar3algofactory() ;
   virtual lifealgo *createInstance() {
      return new bitpar3algo() ;
   }
} factory ;
bitpar3algofactory::bitpar3algofactory() {
   registerAlgo("bitpar3", &factory) ;
}
void bitpar3algo::init(int w_, int h_) {
   w = w_ ;
   h = h_ ;
   wordwidth = (w + 63) >> 6 ;
   wh = wordwidth * h ;
   u0 = (ull *)calloc(wordwidth*sizeof(ull), h+1) ;
   u1 = (ull *)calloc(wordwidth*sizeof(ull), h+1) ;
}
void bitpar3algo::setcell(int x, int y) {
   u0[(x>>6)*h+y] |= 1LL << (x & 63) ;
}
int bitpar3algo::getpopulation() {
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
void bitpar3algo::swap() { ::swap(u0, u1) ; }
int bitpar3algo::nextstep(int i, int n, int needpop) {
   int r = 0 ;
   int loi = i * wordwidth / n ;
   int hii = (i + 1) * wordwidth / n ;
   for (int i=loi; i<hii; i++) {
      ull w300 = 0 ;
      ull w301 = 0 ;
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
      ull w210, w211, w310, w311 ;
      add2(w1l, w1r, w210, w211) ;
      add2(w1, w210, w310, w311) ;
      w311 |= w211 ;
#pragma unroll 8
      for (int j=1; j+1<h; j++, col++, pcol++, ncol++, wcol++) {
         ull w2 = col[1] ;
         ull w2l = w2 << 1 ;
         ull w2r = w2 >> 1 ;
         if (i > 0)
            w2l |= pcol[1] >> 63 ;
         if (i+1 < wordwidth)
            w2r |= ncol[1] << 63 ;
         ull w220, w221, w320, w321, a0, a1 ;
         add2(w2l, w2r, w220, w221) ;
         add2(w2, w220, w320, w321) ;
         w321 |= w221 ;
         add3(w300, w210, w320, a0, a1) ;
         ull ng1 = (a1 ^ w301 ^ w211 ^ w321) & ((a1 | w301) ^ (w211 | w321)) &
                   (a0 | w1) ;
         wcol[0] = ng1 ;
         if (needpop)
            r += __builtin_popcountll(ng1) ;
         w300 = w310 ;
         w301 = w311 ;
         w310 = w320 ;
         w311 = w321 ;
         w210 = w220 ;
         w211 = w221 ;
         w1 = w2 ;
      }
   }
   return r ;
}
