#include "lifealgo.h"
#include <algorithm>
#include <cstdlib>
using namespace std ;
class array2algo : public lifealgo {
public:
   virtual void init(int w, int h) ;
   virtual void setcell(int x, int y) ;
   virtual int getpopulation() ;
   virtual int nextstep(int, int) ;
   virtual void swap() ;
   int w, h ;
   long long wh ;
   unsigned char *u0, *u1 ;
} ;
static class array2algofactory : public lifealgofactory {
public:
   array2algofactory() ;
   virtual lifealgo *createInstance() {
      return new array2algo() ;
   }
} factory ;
array2algofactory::array2algofactory() {
   registerAlgo("array2", &factory) ;
}
void array2algo::init(int w_, int h_) {
   w = w_ ;
   h = h_ ;
   wh = w * h ;
   u0 = (unsigned char *)calloc(w, h) ;
   u1 = (unsigned char *)calloc(w, h) ;
}
void array2algo::setcell(int x, int y) {
   u0[y * w + x] = 1 ;
}
int array2algo::getpopulation() {
   int r = 0 ;
   for (int i=0; i<wh; i++)
      r += u0[i] ;
   return r ;
}
void array2algo::swap() { ::swap(u0, u1) ; }
int array2algo::nextstep(int id, int nid) {
   int pop = 0 ;
   int ylo = (h - 2) * id / nid + 1 ;
   int yhi = (h - 2) * (id + 1) / nid + 1 ;
   for (int y=ylo; y<yhi; y++) {
      unsigned char *rp = u0 + (y-1)*w + 1 ;
      unsigned char *r = u0 + y*w + 1 ;
      unsigned char *rn = u0 + (y+1)*w + 1 ;
      unsigned char *wr = u1 + y*w + 1 ;
      for (int x=1; x+1<w; x++, r++, rp++, rn++, wr++) {
         int n = rp[-1]+rp[0]+rp[1]+r[-1]+r[1]+rn[-1]+rn[0]+rn[1] ;
         *wr = (n == 3 || (n == 2 && *r)) ;
         pop += *wr ;
      }
   }
   return pop ;
}
