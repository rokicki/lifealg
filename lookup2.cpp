#include "lifealgo.h"
#include <algorithm>
#include <cstdlib>
using namespace std ;
class lookup2algo : public lifealgo {
public:
   virtual void init(int w, int h) ;
   virtual void setcell(int x, int y) ;
   virtual int getpopulation() ;
   virtual int nextstep(int, int, int) ;
   virtual void swap() ;
   int w, h ;
   long long wh ;
   unsigned char *u0, *u1 ;
} ;
static class lookup2algofactory : public lifealgofactory {
public:
   lookup2algofactory() ;
   virtual lifealgo *createInstance() {
      return new lookup2algo() ;
   }
} factory ;
lookup2algofactory::lookup2algofactory() {
   registerAlgo("lookup2", &factory) ;
}
void lookup2algo::init(int w_, int h_) {
   w = w_ ;
   h = h_ ;
   wh = w * h ;
   u0 = (unsigned char *)calloc(w, h) ;
   u1 = (unsigned char *)calloc(w, h) ;
}
void lookup2algo::setcell(int x, int y) {
   u0[y * w + x] = 1 ;
}
int lookup2algo::getpopulation() {
   int r = 0 ;
   for (int i=0; i<wh; i++)
      r += u0[i] ;
   return r ;
}
static unsigned char tab[] =
    { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 } ;
void lookup2algo::swap() { ::swap(u0, u1) ; }
int lookup2algo::nextstep(int id, int n, int) {
   int pop = 0 ;
   int loy = id * (h - 2) / n + 1 ;
   int hiy = (id + 1) * (h - 2) / n + 1 ;
   for (int y=loy; y<hiy; y++) {
      unsigned char *rp = u0 + (y-1)*w + 1 ;
      unsigned char *r = u0 + y*w + 1 ;
      unsigned char *rn = u0 + (y+1)*w + 1 ;
      unsigned char *wr = u1 + y*w + 1 ;
      for (int x=1; x+1<w; x++, r++, rp++, rn++, wr++) {
         int n = tab[rp[-1]+rp[0]+rp[+1]+r[-1]+r[1]+rn[-1]+rn[0]+rn[1]+9* *r] ;
         pop += n ;
         *wr = n ;
      }
   }
   return pop ;
}
