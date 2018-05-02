#ifdef USEDBLIFE
#include "lifealgo.h"
#include <algorithm>
#include <cstdlib>
using namespace std ;
extern "C" {
   void tgrdb_setIncrement(int) ;
   void tgrdb_init(const char *) ;
   void tgrdb_setcell(int, int) ;
   int tgrdb_nextstep() ;
   int tgrdb_getpopulation() ;
} ;
class dbquadalgo : public lifealgo {
public:
   virtual void init(int w, int h) {
      tgrdb_init("q") ;
   }
   virtual void setcell(int x, int y) {
      tgrdb_setcell(x, y) ;
   }
   virtual int getpopulation() {
      return tgrdb_getpopulation() ;
   }
   virtual int nextstep(int, int, int) {
      return tgrdb_nextstep() ;
   }
   virtual int nextstep() {
      return tgrdb_nextstep() ;
   }
   virtual void setinc(int inc) {
      lifealgo::setinc(inc) ;
      tgrdb_setIncrement(inc) ;
   }
   virtual void swap() {}
   int w, h ;
   long long wh ;
   unsigned char *u0, *u1 ;
} ;
static class dbquadalgofactory : public lifealgofactory {
public:
   dbquadalgofactory() ;
   virtual lifealgo *createInstance() {
      return new dbquadalgo() ;
   }
} factory ;
dbquadalgofactory::dbquadalgofactory() {
   registerAlgo("dbquad", &factory) ;
}
#endif
