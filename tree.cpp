#include "lifealgo.h"
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <algorithm>
using namespace std ;
typedef unsigned long long ull ;
union node {
   node *c[4] ;
   ull leaf[4] ;
} ;
const int BLOCKSIZE = 1021 ; // how many nodes we allocate at once
class treealgo : public lifealgo {
public:
   treealgo() : root(0), blocks(0), freenodes(0) {}
   virtual void init(int w, int h) ;
   virtual void setcell(int x, int y) ;
   virtual int getpopulation() ;
   virtual int nextstep(int, int) ;
   virtual void swap() ;
   node *getnode() {
      if (freenodes == 0) {
         node *b = (node *)calloc(sizeof(node), BLOCKSIZE+1) ;
         b->c[0] = blocks ;
         freenodes = b + 1 ;
         for (int i=1; i<BLOCKSIZE; i++)
            b[i].c[0] = b+i+1 ;
      }
      node *r = freenodes ;
      freenodes = r->c[0] ;
      return r ;
   }
   void freenode(node *n) {
      n->c[0] = freenodes ;
      freenodes = n ;
   }
   void freetree(node *n, int d) {
      if (n == 0)
         return ;
      if (d > 4)
         for (int i=0; i<4; i++)
            freetree(n->c[i], d-1) ;
      freenode(n) ;
   }
   node *makenode(node *nw, node *ne, node *sw, node *se) {
      node *r = getnode() ;
      r->c[0] = nw ;
      r->c[1] = ne ;
      r->c[2] = sw ;
      r->c[3] = se ;
      return r ;
   }
   void pushroot() {
      node *newroot = makenode(
         makenode(0, 0, 0, root->c[0]), makenode(0, 0, root->c[1], 0),
         makenode(0, root->c[2], 0, 0), makenode(root->c[3], 0, 0, 0)) ;
      freenode(root) ;
      root = newroot ;
      depth++ ;
   }
   void poproot() {
      while (depth > 4) {
         node *n = root->c[0] ;
         if (n->c[0] || n->c[1] || n->c[2])
            return ;
         n = root->c[1] ;
         if (n->c[0] || n->c[1] || n->c[3])
            return ;
         n = root->c[2] ;
         if (n->c[0] || n->c[2] || n->c[3])
            return ;
         n = root->c[3] ;
         if (n->c[1] || n->c[2] || n->c[3])
            return ;
         n = makenode(root->c[0]->c[3], root->c[1]->c[2],
                      root->c[2]->c[1], root->c[3]->c[0]) ;
         for (int i=0; i<4; i++)
            freenode(root->c[i]) ;
         root = n ;
         depth-- ;
      }
   }
   void setcell(int x, int y, node *n, int d) ;
   int getpop(node *n, int d) ;
   node *root, *blocks, *freenodes ;
   int depth = 0 ;
} ;
static class treealgofactory : public lifealgofactory {
public:
   treealgofactory() ;
   virtual lifealgo *createInstance() {
      return new treealgo() ;
   }
} factory ;
treealgofactory::treealgofactory() {
   registerAlgo("tree", &factory) ;
}
static int popcount64(ull v) {
   return __builtin_popcount(v) + __builtin_popcount(v >> 32) ;
}
void treealgo::init(int w_, int h_) {
   depth = 4 ;
   root = makenode(0, 0, 0, 0) ;
}
void treealgo::setcell(int x, int y, node *n, int d) {
   int off = ((x >> (d - 1)) & 1) + ((y >> (d - 2)) & 2) ;
   if (d == 4) {
      n->leaf[off] |= 1LL << ((x & 7) + ((y & 7) << 3)) ;
   } else {
      if (n->c[off] == 0)
         n->c[off] = makenode(0, 0, 0, 0) ;
      setcell(x, y, n->c[off], d-1) ;
   }
}
void treealgo::setcell(int x, int y) {
   while ((x >> (depth-1)) > 0 || (x >> (depth-1)) < -1 ||
          (y >> (depth-1)) > 0 || (y >> (depth-1)) < -1)
      pushroot() ;
   int w = 1 << (depth - 1) ;
   setcell(x + w, y + w, root, depth) ;
}
int treealgo::getpop(node *n, int d) {
   if (n == 0)
      return 0 ;
   if (d == 4)
      return popcount64(n->leaf[0]) + popcount64(n->leaf[1]) +
             popcount64(n->leaf[2]) + popcount64(n->leaf[3]) ;
   return getpop(n->c[0], d-1) + getpop(n->c[1], d-1) +
          getpop(n->c[2], d-1) + getpop(n->c[3], d-1) ;
}
int treealgo::getpopulation() {
   return getpop(root, depth) ;
}
void treealgo::swap() { }
int treealgo::nextstep(int id, int nid) {
   if (nid != 1)
      error("! multithreading not yet supported") ;
   return getpopulation() ;
}
