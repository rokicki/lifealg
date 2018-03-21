#include "lifealgo.h"
#include <algorithm>
#include <cstdlib>
using namespace std ;
class arrayalgo : public lifealgo {
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
static class arrayalgofactory : public lifealgofactory {
public:
   arrayalgofactory() ;
   virtual lifealgo *createInstance() {
      return new arrayalgo() ;
   }
} factory ;
arrayalgofactory::arrayalgofactory() {
   registerAlgo("array", &factory) ;
}
void arrayalgo::init(int w_, int h_) {
   w = w_ ;
   h = h_ ;
   wh = w * h ;
   u0 = (unsigned char *)calloc(w, h) ;
   u1 = (unsigned char *)calloc(w, h) ;
}
void arrayalgo::setcell(int x, int y) {
   u0[y * w + x] = 1 ;
}
int arrayalgo::getpopulation() {
   int r = 0 ;
   for (int i=0; i<wh; i++)
      r += u0[i] ;
   return r ;
}
void arrayalgo::swap() { ::swap(u0, u1) ; }
int arrayalgo::nextstep(int id, int nid) {
   int pop = 0 ;
   int ylo = (h - 2) * id / nid + 1 ;
   int yhi = (h - 2) * (id + 1) / nid + 1 ;
   for (int y=ylo; y<yhi; y++) {
      unsigned char *r = u0 + y*w + 1 ;
      unsigned char *wr = u1 + y*w + 1 ;
      for (int x=1; x+1<w; x++, r++, wr++) {
         int n = r[-w-1]+r[-w]+r[-w+1]+r[-1]+r[1]+r[w-1]+r[w]+r[w+1] ;
         *wr = (n == 3 || (n == 2 && *r)) ;
         pop += *wr ;
      }
   }
   return pop ;
}
