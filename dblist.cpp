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
class dblistalgo : public lifealgo {
public:
   virtual void init(int w, int h) {
      tgrdb_init("l") ;
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
static class dblistalgofactory : public lifealgofactory {
public:
   dblistalgofactory() ;
   virtual lifealgo *createInstance() {
      return new dblistalgo() ;
   }
} factory ;
dblistalgofactory::dblistalgofactory() {
   registerAlgo("dblist", &factory) ;
}
#endif
