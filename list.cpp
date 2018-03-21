#include "lifealgo.h"
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <algorithm>
using namespace std ;
class listalgo : public lifealgo {
public:
   virtual void init(int w, int h) ;
   virtual void setcell(int x, int y) ;
   virtual int getpopulation() ;
   virtual int nextstep(int, int, int) ;
   virtual void swap() ;
   int sorted ;
   vector<pair<int, int> > pts, pts2 ;
} ;
static class listalgofactory : public lifealgofactory {
public:
   listalgofactory() ;
   virtual lifealgo *createInstance() {
      return new listalgo() ;
   }
} factory ;
listalgofactory::listalgofactory() {
   registerAlgo("list", &factory) ;
}
void listalgo::init(int w_, int h_) {
}
void listalgo::setcell(int x, int y) {
   sorted = 0 ;
   pts.push_back(make_pair(x, y)) ;
}
int listalgo::getpopulation() {
   sorted = 0 ;
   return pts.size() ;
}
void listalgo::swap() { }
int listalgo::nextstep(int id, int nid, int) {
   if (nid != 1)
      error("! multithreading not yet supported") ;
   pts2.clear() ;
   for (size_t i=0; i<pts.size(); i++) {
      int x = pts[i].first ;
      int y = pts[i].second ;
      pts2.push_back(make_pair(x-1, y-1)) ;
      pts2.push_back(make_pair(x-1, y)) ;
      pts2.push_back(make_pair(x-1, y+1)) ;
      pts2.push_back(make_pair(x, y-1)) ;
      pts2.push_back(make_pair(x, y+1)) ;
      pts2.push_back(make_pair(x+1, y-1)) ;
      pts2.push_back(make_pair(x+1, y)) ;
      pts2.push_back(make_pair(x+1, y+1)) ;
   }
   sort(pts2.begin(), pts2.end()) ;
   if (!sorted) {
      sort(pts.begin(), pts.end()) ;
      sorted = 1 ;
   }
   size_t w = 0 ;
   size_t rd = 0 ;
   size_t at = 0 ;
   while (at < pts2.size()) {
      int cnt = 1 ;
      while (at + cnt < pts2.size() && pts2[at+cnt] == pts2[at])
         cnt++ ;
      while (rd < pts.size() && pts[rd] < pts2[at])
         rd++ ;
      if (cnt == 3 || (cnt == 2 && rd < pts.size() && pts[rd] == pts2[at]))
         pts2[w++] = pts2[at] ;
      at += cnt ;
   }
   pts2.resize(w) ;
   ::swap(pts, pts2) ;
   return pts.size() ;
}
