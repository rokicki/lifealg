#ifndef LIFEALGO_H
#define LIFEALGO_H
#include "util.h"
class lifealgo {
public:
   lifealgo() { increment = 1 ; }
   virtual void init(int w, int h) = 0 ;
   virtual void setcell(int x, int y) = 0 ;
   virtual int getpopulation() = 0 ;
   virtual void swap() = 0 ;
   virtual int nextstep(int i, int n, int needpop) = 0 ;
   virtual void setinc(int inc) { increment = inc ; }
   virtual int nextstep() {
      for (int i=1; i<increment; i++) {
         nextstep(0, 1, 0) ;
         swap() ;
      }
      int r = nextstep(0, 1, 1) ;
      swap() ;
      return r ;
   }
   int increment ;
} ;
class lifealgofactory {
public:
   virtual lifealgo *createInstance() = 0 ;
} ;
extern void registerAlgo(const char *name, lifealgofactory *laf) ;
extern void readrle(const char *filename, lifealgo *la, int startx, int starty) ;
#endif
