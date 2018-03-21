#include "lifealgo.h"
#include <algorithm>
#include <cstdlib>
using namespace std ;
class array4algo : public lifealgo {
public:
   virtual void init(int w, int h) ;
   virtual void setcell(int x, int y) ;
   virtual int getpopulation() ;
   virtual int nextstep(int, int) ;
   virtual void swap() ;
   int w, h ;
   long long wh ;
} ;
const int MAXDIMEN = 8192 ;
static unsigned char u0[MAXDIMEN][MAXDIMEN] ;
static unsigned char u1[MAXDIMEN][MAXDIMEN] ;
static class array4algofactory : public lifealgofactory {
public:
   array4algofactory() ;
   virtual lifealgo *createInstance() {
      return new array4algo() ;
   }
} factory ;
array4algofactory::array4algofactory() {
   registerAlgo("array4", &factory) ;
}
void array4algo::init(int w_, int h_) {
   w = w_ ;
   h = h_ ;
   wh = w * h ;
}
void array4algo::setcell(int x, int y) {
   u0[y][x] = 1 ;
}
int array4algo::getpopulation() {
   int r = 0 ;
   for (int y=1; y+1<h; y++)
      for (int x=1; x+1<w; x++)
         r += u0[y][x] ;
   return r ;
}
void array4algo::swap() {}
int array4algo::nextstep(int id, int nid) {
   if (nid != 1)
      error("! multithreading not supported yet") ;
   int pop = 0 ;
   for (int i=0; i<=h; i++)
      u1[i][0] = 0 ;
   for (int j=0; j<=w; j++)
      u1[0][j] = 0 ;
   for (int i=0; i<h; i++)
      for (int j=0; j<w; j++)
         u1[i+1][j+1] = u0[i][j] + u1[i][j+1] + u1[i+1][j] - u1[i][j] ;
   for (int i=1; i+1<h; i++)
      for (int j=1; j+1<w; j++) {
         unsigned char n = u1[i+2][j+2] - u1[i+2][j-1] - u1[i-1][j+2] + u1[i-1][j-1] ;
         if (n == 3 || (n == 4 && u0[i][j])) {
            u0[i][j] = 1 ;
            pop++ ;
         } else
            u0[i][j] = 0 ;
      }
   return pop ;
}
