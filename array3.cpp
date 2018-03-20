#include "lifealgo.h"
#include <algorithm>
#include <cstdlib>
using namespace std ;
class array3algo : public lifealgo {
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
static class array3algofactory : public lifealgofactory {
public:
   array3algofactory() ;
   virtual lifealgo *createInstance() {
      return new array3algo() ;
   }
} factory ;
array3algofactory::array3algofactory() {
   registerAlgo("array3", &factory) ;
}
void array3algo::init(int w_, int h_) {
   w = w_ ;
   h = h_ ;
   wh = w * h ;
   u0 = (unsigned char *)calloc(w, h+2) ;
   u1 = (unsigned char *)calloc(w, h+2) + w ;
}
void array3algo::setcell(int x, int y) {
   u0[y * w + x] = 1 ;
}
int array3algo::getpopulation() {
   int r = 0 ;
   for (int i=0; i<wh; i++)
      r += u0[i] ;
   return r ;
}
void array3algo::swap() {}
#include <iostream>
int array3algo::nextstep(int id, int nid) {
   if (nid != 1)
      error("! multithreading not supported yet") ;
   int pop = 0 ;
   u1[0] = 0 ;
   for (int i=1; i<=wh+w; i++)
      u1[i] = u1[i-1] + u0[i-1] ;
   for (int i=0; i<wh; i++)
      u1[i+w] += u1[i] ;
   for (int y=1; y+1<h; y++) {
      for (int x=1; x+1<w; x++) {
         unsigned char n = u1[(y+1)*w+(x+2)] - u1[(y+1)*w+(x-1)] -
                           u1[(y-2)*w+(x+2)] + u1[(y-2)*w+(x-1)] ;
         if (n == 3 || (n == 4 && u0[y*w+x])) {
            pop++ ;
            u0[y*w+x] = 1 ;
         } else {
            u0[y*w+x] = 0 ;
         }
      }
   }
   return pop ;
}
