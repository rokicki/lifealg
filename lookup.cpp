#include "lifealgo.h"
#include <algorithm>
#include <cstdlib>
using namespace std ;
class lookupalgo : public lifealgo {
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
static class lookupalgofactory : public lifealgofactory {
public:
   lookupalgofactory() ;
   virtual lifealgo *createInstance() {
      return new lookupalgo() ;
   }
} factory ;
lookupalgofactory::lookupalgofactory() {
   registerAlgo("lookup", &factory) ;
}
void lookupalgo::init(int w_, int h_) {
   w = w_ ;
   h = h_ ;
   wh = w * h ;
   u0 = (unsigned char *)calloc(w, h) ;
   u1 = (unsigned char *)calloc(w, h) ;
}
void lookupalgo::setcell(int x, int y) {
   u0[y * w + x] = 1 ;
}
int lookupalgo::getpopulation() {
   int r = 0 ;
   for (int i=0; i<wh; i++)
      r += u0[i] ;
   return r ;
}
static unsigned char tab[] =
    { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 } ;
void lookupalgo::swap() { ::swap(u0, u1) ; }
int lookupalgo::nextstep(int id, int n) {
   int pop = 0 ;
   int loy = id * (h - 2) / n + 1 ;
   int hiy = (id + 1) * (h - 2) / n + 1 ;
   for (int y=loy; y<hiy; y++) {
      unsigned char *r = u0 + y*w + 1 ;
      unsigned char *wr = u1 + y*w + 1 ;
      for (int x=1; x+1<w; x++, r++, wr++) {
         int n = tab[r[-w-1]+r[-w]+r[-w+1]+r[-1]+r[1]+r[w-1]+r[w]+r[w+1]+9* *r] ;
         pop += n ;
         *wr = n ;
      }
   }
   return pop ;
}
