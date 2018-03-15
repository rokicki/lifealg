#include "lifealgo.h"
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <algorithm>
using namespace std ;
const int END = 0x7ffffff0 ;
class list3algo : public lifealgo {
public:
   virtual void init(int w, int h) ;
   virtual void setcell(int x, int y) ;
   virtual int getpopulation() ;
   virtual int nextstep(int, int) ;
   virtual void swap() ;
   void createit() ;
   int created ;
   vector<pair<int, int> > pts ;
   vector<int> a0, a1 ;
} ;
static class list3algofactory : public lifealgofactory {
public:
   list3algofactory() ;
   virtual lifealgo *createInstance() {
      return new list3algo() ;
   }
} factory ;
list3algofactory::list3algofactory() {
   registerAlgo("list3", &factory) ;
}
void list3algo::init(int w_, int h_) {
}
void list3algo::setcell(int x, int y) {
   created = 0 ;
   pts.push_back(make_pair(y, x)) ;
}
int list3algo::getpopulation() {
   if (!created)
      return pts.size() ;
   int r = 0 ;
   int at = 0 ;
   while (a0[at] != END) { // for each row
      at++ ;
      while (a0[at] != END) { // for each point 
         at++ ;
         r++ ;
      }
      at++ ;
   }
   return r ;
}
void list3algo::swap() { }
void list3algo::createit() {
   if (!created) {
      sort(pts.begin(), pts.end()) ;
      int cury = -1 ;
      for (size_t i=0; i<pts.size(); i++) {
         if (pts[i].first != cury) {
            if (cury != -1)
               a0.push_back(END) ;
            cury = pts[i].first ;
            a0.push_back(cury) ;
         }
         a0.push_back(pts[i].second) ;
      }
      a0.push_back(END) ;
      if (cury != -1)
         a0.push_back(END) ;
      created = 1 ;
   }
}
int list3algo::nextstep(int id, int nid) {
   int r = 0 ;
   if (nid != 1)
      error("! multithreading not yet supported") ;
   if (!created)
      createit() ;
   a1.clear() ;
   int r0=0, r1=0, r2=0 ;
   int wy = END ;
   while (a0[r0] != END) {
      int y = min(a0[r0]+1, min(a0[r1], a0[r2]-1)) ;
      int p0 = r0+1 ;
      int p1 = r1 ;
      int p2 = r2 ;
      if (a0[p1] != END)
         p1++ ;
      if (a0[p2] != END)
         p2++ ;
      int x = -END ;
      while (1) {
         int xt = END ;
         if (a0[r0] == y-1)
            xt = a0[p0] ;
         if (a0[r1] == y)
            xt = min(xt, a0[p1]) ;
         if (a0[r2] == y+1)
            xt = min(xt, a0[p2]) ;
         if (xt == END) {
            if (a0[r0] == y-1)
               r0 = p0 + 1 ;
            if (a0[r1] == y)
               r1 = p1 + 1 ;
            if (a0[r2] == y+1)
               r2 = p2 + 1 ;
            break ;
         }
         x = max(x, xt-1) ;
         int n = 0 ;
         if (a0[r0] == y-1) {
            for (int i=0; a0[p0+i] < x+2; i++)
               n++ ;
            if (a0[p0] == x-1)
               p0++ ;
         }
         if (a0[r1] == y) {
            for (int i=0; a0[p1+i] < x+2; i++)
               n++ ;
            if (a0[p1] == x-1)
               p1++ ;
         }
         if (a0[r2] == y+1) {
            for (int i=0; a0[p2+i] < x+2; i++)
               n++ ;
            if (a0[p2] == x-1)
               p2++ ;
         }
         if (n == 3 || (n == 4 && a0[r1] == y && a0[p1] == x)) {
            if (wy != y) {
               if (wy != END)
                  a1.push_back(END) ;
               wy = y ;
               a1.push_back(y) ;
            }
            a1.push_back(x) ;
            r++ ;
         }
         x++ ;
      }
   }
   a1.push_back(END) ;
   if (wy != END)
      a1.push_back(END) ;
   ::swap(a0, a1) ;
   return r ;
}
