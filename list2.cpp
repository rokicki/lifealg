#include "lifealgo.h"
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <limits.h>
using namespace std ;
class list2algo : public lifealgo {
public:
   virtual void init(int w, int h) ;
   virtual void setcell(int x, int y) ;
   virtual int getpopulation() ;
   virtual int nextstep(int, int) ;
   virtual void swap() ;
   int sorted ;
   vector<pair<int, int> > pts, pts2 ;
} ;
static class list2algofactory : public lifealgofactory {
public:
   list2algofactory() ;
   virtual lifealgo *createInstance() {
      return new list2algo() ;
   }
} factory ;
list2algofactory::list2algofactory() {
   registerAlgo("list2", &factory) ;
}
void list2algo::init(int w_, int h_) {
}
void list2algo::setcell(int x, int y) {
   sorted = 0 ;
   pts.push_back(make_pair(x, y)) ;
}
int list2algo::getpopulation() {
   sorted = 0 ;
   return pts.size() ;
}
void list2algo::swap() { }
int list2algo::nextstep(int id, int nid) {
   if (nid != 1)
      error("! multithreading not yet supported") ;
   if (!sorted) {
      sort(pts.begin(), pts.end()) ;
      sorted = 1 ;
   }
   pts2.clear() ;
   if (pts.size() > 0) {
      int x = pts[0].first - 1 ;
      int y = pts[0].second - 1 ;
      int ptrs[3] ;
      for (int i=0; i<3; i++)
         ptrs[i] = 0 ;
      while (ptrs[0] < pts.size()) {
         int n = 0 ;
         for (int i=0; i<3; i++) {
            pair<int, int> leftedge = make_pair(x-1+i, y-1) ;
            pair<int, int> rightedge = make_pair(x-1+i, y+2) ;
            int cnt = 0 ;
            while (ptrs[i] + cnt < pts.size() && pts[ptrs[i]+cnt] < rightedge)
               cnt++ ;
            n += cnt ;
            if (ptrs[i] < pts.size() && pts[ptrs[i]] == leftedge) {
               ptrs[i]++ ;
            }
         }
         if (n == 3 || (n == 4 && ptrs[1] < pts.size() &&
                        pts[ptrs[1]].first == x && 
                        pts[ptrs[1]].second == y))
            pts2.push_back(make_pair(x, y)) ;
         if (n == 0) {
            x = INT_MAX ;
            y = INT_MAX ;
            for (int i=0; i<3; i++) {
               if (ptrs[i] < pts.size() &&
                   (pts[ptrs[i]].first + 1 - i < x ||
                    (pts[ptrs[i]].first + 1 - i == x &&
                     pts[ptrs[i]].second-1 < y))) {
                  x = pts[ptrs[i]].first + 1 - i ;
                  y = pts[ptrs[i]].second-1 ;
               }
            }
         } else {
            y++ ;
         }
      }
   }
   ::swap(pts, pts2) ;
   return pts.size() ;
}
