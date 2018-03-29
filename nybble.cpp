#include "lifealgo.h"
#include "util.h"
#include <algorithm>
#include <cstdlib>
using namespace std ;
typedef unsigned long long ull ;
class nybblealgo : public lifealgo {
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
static class nybblealgofactory : public lifealgofactory {
public:
   nybblealgofactory() ;
   virtual lifealgo *createInstance() {
      return new nybblealgo() ;
   }
} factory ;
nybblealgofactory::nybblealgofactory() {
   registerAlgo("nybble", &factory) ;
}
void nybblealgo::init(int w_, int h_) {
   w = w_ ;
   h = h_ ;
   wordwidth = (w + 15) >> 4 ;
   wh = wordwidth * h ;
   u0 = (ull *)calloc(wordwidth*sizeof(ull), h+1) ;
   u1 = (ull *)calloc(wordwidth*sizeof(ull), h+1) ;
}
void nybblealgo::setcell(int x, int y) {
   u0[(x >> 4) + wordwidth * y] |= 1LL << (4 * (x & 15)) ;
}
int nybblealgo::getpopulation() {
   int r = 0 ;
   for (int i=0; i<wh; i++)
      r += __builtin_popcountll(u0[i]) ;
   return r ;
}
void nybblealgo::swap() { ::swap(u0, u1) ; }
int nybblealgo::nextstep(int id, int n, int needpop) {
   ull r = 0 ;
   int loi = id * (h - 2) / n + 1 ;
   int hii = (id + 1) * (h - 2) / n + 1 ;
   for (int i=loi; i<hii; i++) {
      ull *pwp = u0 + wordwidth * (i - 1) ;
      ull *cwp = u0 + wordwidth * i ;
      ull *nwp = u0 + wordwidth * (i + 1) ;
      ull *wr = u1 + wordwidth * i ;
      ull pws = 0 ;
      ull cws = *pwp + *cwp + *nwp ;
      for (int j=0; j<wordwidth; j++, pwp++, cwp++, nwp++, wr++) {
         ull nws = 0 ;
         if (j+1 < wordwidth)
            nws = pwp[1] + cwp[1] + nwp[1] ;
         ull cw = *cwp ;
         ull ng = cw | ((pws >> 60) + (cws >> 4) + cws + (cws << 4) + (nws << 60) - cw) ;
         ng &= (ng >> 1) & (~((ng >> 2) | (ng >> 3))) & 0x1111111111111111LL ;
         if (needpop)
            r += __builtin_popcountll(ng) ;
         *wr = ng ;
         pws = cws ;
         cws = nws ;
      }
   }
   return (int)r ;
}
