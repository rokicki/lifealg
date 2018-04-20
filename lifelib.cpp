#include "pattern2.h"
#include "upattern.h"
#include "lifealgo.h"
#include <vector>
#include <algorithm>
#include <cstdlib>
using namespace std ;
const int memsize = 1000 ;
class lifelibalgo : public lifealgo {
public:
   lifelibalgo() : lt(memsize), patbuilt(0), pat(&lt, "") { }
   virtual void init(int w, int h) ;
   virtual void setcell(int x, int y) ;
   virtual int getpopulation() ;
   virtual int nextstep(int, int, int) ;
   virtual void swap() ;
   virtual int nextstep() { 
      return nextstep(0, 1, 1) ;
   }
   void buildpat() ;
   apg::lifetree<uint32_t, 1> lt ;
   apg::bitworld bw ;
   int patbuilt ;
   apg::pattern pat ;
} ;
static class lifelibalgofactory : public lifealgofactory {
public:
   lifelibalgofactory() ;
   virtual lifealgo *createInstance() {
      return new lifelibalgo() ;
   }
} factory ;
lifelibalgofactory::lifelibalgofactory() {
   registerAlgo("lifelib", &factory) ;
}
void lifelibalgo::init(int w_, int h_) { }
void lifelibalgo::setcell(int x, int y) {
   bw.setcell(x, y, 1) ;
}
void lifelibalgo::buildpat() {
   vector<apg::bitworld> planes ;
   planes.push_back(bw) ;
   pat = apg::pattern(&lt, planes, "b3s23") ;
   patbuilt = 1 ;
}
int lifelibalgo::getpopulation() {
   if (!patbuilt)
      buildpat() ;
   return pat.popcount(1000000009) ;
}
void lifelibalgo::swap() { }
int lifelibalgo::nextstep(int id, int nid, int needpop) {
   if (nid != 1)
      error("! multithreading not supported") ;
   pat = pat[increment] ;
   return getpopulation() ;
}
typedef apg::upattern<apg::VTile28, 28> upat ;
class ulifelibalgo : public lifealgo {
public:
   ulifelibalgo() : patbuilt(0), pat() { }
   virtual void init(int w, int h) ;
   virtual void setcell(int x, int y) ;
   virtual int getpopulation() ;
   virtual int nextstep(int, int, int) ;
   virtual void swap() ;
   virtual void setinc(int inc) {
      lifealgo::setinc(inc) ;
      if (inc & 1)
         error("! odd increments not supported") ;
   }
   virtual int nextstep() { 
      return nextstep(0, 1, 1) ;
   }
   void buildpat() ;
   apg::bitworld bw ;
   int patbuilt ;
   upat pat ;
} ;
static class ulifelibalgofactory : public lifealgofactory {
public:
   ulifelibalgofactory() ;
   virtual lifealgo *createInstance() {
      return new ulifelibalgo() ;
   }
} ufactory ;
ulifelibalgofactory::ulifelibalgofactory() {
   registerAlgo("ulifelib", &ufactory) ;
}
void ulifelibalgo::init(int w_, int h_) { }
void ulifelibalgo::setcell(int x, int y) {
   bw.setcell(x, y, 1) ;
}
void ulifelibalgo::buildpat() {
   vector<apg::bitworld> planes ;
   planes.push_back(bw) ;
   pat.insertPattern(planes) ;
   patbuilt = 1 ;
}
int ulifelibalgo::getpopulation() {
   if (!patbuilt)
      buildpat() ;
   return pat.totalPopulation() ;
}
void ulifelibalgo::swap() { }
int ulifelibalgo::nextstep(int id, int nid, int needpop) {
   if (nid != 1)
      error("! multithreading not supported") ;
   if (increment & 1)
      error("! odd increments not supported") ;
   pat.advance(0, 0, increment) ;
   if (needpop)
      return getpopulation() ;
   else
      return 0 ;
}
