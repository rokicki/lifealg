#ifndef LIFEALGO_H
#define LIFEALGO_H
#include "util.h"
class lifealgo {
public:
   virtual void init(int w, int h) = 0 ;
   virtual void setcell(int x, int y) = 0 ;
   virtual int getpopulation() = 0 ;
   virtual void swap() = 0 ;
   virtual int nextstep(int i, int n) = 0 ;
   int nextstep() {
      int r = nextstep(0, 1) ;
      swap() ;
      return r ;
   }
} ;
class lifealgofactory {
public:
   virtual lifealgo *createInstance() = 0 ;
} ;
extern void registerAlgo(const char *name, lifealgofactory *laf) ;
#endif
