#include "lifealgo.h"
#include "util.h"
#include <algorithm>
#include <cstdlib>
using namespace std ;
typedef unsigned long long ull ;
class lookup4algo : public lifealgo {
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
static class lookup4algofactory : public lifealgofactory {
public:
   lookup4algofactory() ;
   virtual lifealgo *createInstance() {
      return new lookup4algo() ;
   }
} factory ;
lookup4algofactory::lookup4algofactory() {
   registerAlgo("lookup4", &factory) ;
}
// input is row major order.  Output is 00xx00yy.
static unsigned char lookuptab[1<<16] ;
// slow calculation for input 0bbb0bbb0bbb values.
static int slowcalc(int v) {
   int n = __builtin_popcount(v & 0x777) ;
   if (n == 3 || (n == 4 && (v & 0x20)))
      return 1 ;
   else
      return 0 ;
}
#include <iostream>
void lookup4algo::init(int w_, int h_) {
   w = w_ ;
   h = h_ ;
   wordwidth = (w + 63) >> 6 ;
   wh = wordwidth * h ;
   u0 = (ull *)calloc(wordwidth*sizeof(ull), h+1) ;
   u1 = (ull *)calloc(wordwidth*sizeof(ull), h+1) ;
   for (int i=0; i<65536; i++)
      lookuptab[i] = slowcalc(i) + (slowcalc(i >> 1) << 1) +
                     (slowcalc(i>>4) << 4) + (slowcalc(i>>5) << 5) ;
}
void lookup4algo::setcell(int x, int y) {
   u0[(x>>6)*h+y] |= 1LL << (x & 63) ;
}
int lookup4algo::getpopulation() {
   int r = 0 ;
   for (int i=0; i<wh; i++)
      r += __builtin_popcountll(u0[i]) ;
   return r ;
}
void lookup4algo::swap() { ::swap(u0, u1) ; }
int lookup4algo::nextstep(int i, int n, int needpop) {
   int r = 0 ;
   int loi = i * wordwidth / n ;
   int hii = (i + 1) * wordwidth / n ;
   for (int i=loi; i<hii; i++) {
      ull *pcol = u0 + (i-1) * h + 1 ;
      ull *col = u0 + i * h + 1 ;
      ull *ncol = u0 + (i+1) * h + 1 ;
      ull *wcol = u1 + i * h + 1 ;
      ull p0l = 0 ;
      ull p0r = 0 ;
      ull p1l = *col << 1 ;
      ull p1r = *col >> 1 ;
      if (i > 0)
         p1l += *pcol >> 63 ;
      if (i+1 < wordwidth)
         p1r += *ncol << 63 ;
      for (int j=1; j+1<h; j += 2, col += 2, pcol += 2, ncol += 2, wcol += 2) {
         ull p2l = col[1] << 1 ;
         ull p2r = col[1] >> 1 ;
         ull p3l = col[2] << 1 ;
         ull p3r = col[2] >> 1 ;
         if (i > 0) {
            p2l += pcol[1] >> 63 ;
            p3l += pcol[2] >> 63 ;
         }
         if (i+1 < wordwidth) {
            p2r += ncol[1] << 63 ;
            p3r += ncol[2] << 63 ;
         }
         ull ng1 = 0 ;
         ull ng2 = 0 ;
         for (int k=0; k<64; k += 4) {
            ull bits  = lookuptab[((p0l >> k) & 0xf) +
                                  (((p1l >> k) & 0xf) << 4) +
                                  (((p2l >> k) & 0xf) << 8) +
                                  (((p3l >> k) & 0xf) << 12)] +
                        (lookuptab[((p0r >> k) & 0xf) +
                                  (((p1r >> k) & 0xf) << 4) +
                                  (((p2r >> k) & 0xf) << 8) +
                                  (((p3r >> k) & 0xf) << 12)] << 2) ;
            ng1 += (bits & 0xf) << k ;
            ng2 += (bits >> 4) << k ;
         }
/*
         if (p0l != 0 || p0r != 0 || p1l != 0 || p1r != 0 ||
             p2l != 0 || p2r != 0 || p3l != 0 || p3r != 0) {
  cout << hex << " " << p0l << " " << p0r << " " << p1l << " " << p1r << " " <<
    p2l << " " << p2r << " " << p3l << " " << p3r << " " << ng1 << " " << ng2 <<
    dec << endl ;
         }
 */
         wcol[0] = ng1 ;
         wcol[1] = ng2 ;
         if (needpop)
            r += __builtin_popcountll(ng1) + __builtin_popcountll(ng2) ;
         p0l = p2l ;
         p0r = p2r ;
         p1l = p3l ;
         p1r = p3r ;
      }
   }
   return r ;
}
