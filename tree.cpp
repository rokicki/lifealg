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
   virtual int nextstep(int, int, int) ;
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
      if (n != 0) {
         n->c[0] = freenodes ;
         freenodes = n ;
      }
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
      if (nw == 0 && ne == 0 && sw == 0 && se == 0)
         return 0 ;
      node *r = getnode() ;
      r->c[0] = nw ;
      r->c[1] = ne ;
      r->c[2] = sw ;
      r->c[3] = se ;
      return r ;
   }
   node *makenode(ull nw, ull ne, ull sw, ull se) {
      if (nw == 0 && ne == 0 && sw == 0 && se == 0)
         return 0 ;
      node *r = getnode() ;
      r->leaf[0] = nw ;
      r->leaf[1] = ne ;
      r->leaf[2] = sw ;
      r->leaf[3] = se ;
      return r ;
   }
   node *clearednode() {
      node *r = getnode() ;
      memset(r, 0, sizeof(node)) ;
      return r ;
   }
   void pushroot() {
      if (root != 0) {
         node *newroot = makenode(
            makenode(0, 0, 0, root->c[0]), makenode(0, 0, root->c[1], 0),
            makenode(0, root->c[2], 0, 0), makenode(root->c[3], 0, 0, 0)) ;
         freenode(root) ;
         root = newroot ;
      }
      depth++ ;
   }
   void poproot() {
      while (depth > 4) {
         node *n = root->c[0] ;
         if (n != 0 && (n->c[0] || n->c[1] || n->c[2]))
            return ;
         n = root->c[1] ;
         if (n != 0 && (n->c[0] || n->c[1] || n->c[3]))
            return ;
         n = root->c[2] ;
         if (n != 0 && (n->c[0] || n->c[2] || n->c[3]))
            return ;
         n = root->c[3] ;
         if (n != 0 && (n->c[1] || n->c[2] || n->c[3]))
            return ;
         n = makenode(root->c[0] ? root->c[0]->c[3] : 0,
                      root->c[1] ? root->c[1]->c[2] : 0,
                      root->c[2] ? root->c[2]->c[1] : 0,
                      root->c[3] ? root->c[3]->c[0] : 0) ;
         for (int i=0; i<4; i++)
            freenode(root->c[i]) ;
         freenode(root) ;
         root = n ;
         depth-- ;
      }
   }
   node *calculate(node *nw, node *ne, node *sw, node *se, int d) ;
   void setcell(int x, int y, node *n, int d) ;
   int getpop(node *n, int d) ;
   node *root, *empty, *blocks, *freenodes ;
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
   root = 0 ;
   empty = clearednode() ;
}
void treealgo::setcell(int x, int y, node *n, int d) {
   int off = ((x >> (d - 1)) & 1) + ((y >> (d - 2)) & 2) ;
   if (d == 4) {
      n->leaf[off] |= 1LL << ((x & 7) + ((y & 7) << 3)) ;
   } else {
      if (n->c[off] == 0)
         n->c[off] = clearednode() ;
      setcell(x, y, n->c[off], d-1) ;
   }
}
void treealgo::setcell(int x, int y) {
   while ((x >> (depth-1)) > 0 || (x >> (depth-1)) < -1 ||
          (y >> (depth-1)) > 0 || (y >> (depth-1)) < -1)
      pushroot() ;
   int w = 1 << (depth - 1) ;
   if (root == 0)
      root = clearednode() ;
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
static inline void add2(ull a, ull b,
                        ull &c0, ull &c1) {
   c0 = a ^ b ;
   c1 = a & b ;
}
static inline void add3(ull a, ull b, ull c,
                        ull &c0, ull &c1) {
   ull t0, t1, t2 ;
   add2(a, b, t0, t1) ;
   add2(t0, c, c0, t2) ;
   c1 = t2 | t1 ;
}
/*
 *   Calculate the 8x8 universe one down and right from the 16x16 universe
 *   in the four provided cells; each is an 8x8 universe.  We use bit tricks.
 */
static ull calc(ull nw, ull ne, ull sw, ull se) {
   if ((nw | ne | sw | se) == 0)
      return 0 ;
   ull n0, n1, s0, s1, a0, a1, a2, c1, b1 ;
   add3(nw,
        ((nw >> 1) & 0x7f7f7f7f7f7f7f7fLL) + ((ne << 7) & 0x8080808080808080LL),
        ((nw >> 2) & 0x3f3f3f3f3f3f3f3fLL) + ((ne << 6) & 0xc0c0c0c0c0c0c0c0LL),
        n0, n1) ;
   add3(sw,
        ((sw >> 1) & 0x7f7f7f7f7f7f7f7fLL) + ((se << 7) & 0x8080808080808080LL),
        ((sw >> 2) & 0x3f3f3f3f3f3f3f3fLL) + ((se << 6) & 0xc0c0c0c0c0c0c0c0LL),
        s0, s1) ;
   ull n10 = (n0 >> 8) + (s0 << 56) ;
   ull n11 = (n1 >> 8) + (s1 << 56) ;
   ull n20 = (n0 >> 16) + (s0 << 48) ;
   ull n21 = (n1 >> 16) + (s1 << 48) ;
   add3(n0, n10, n20, a0, c1) ;
   add3(n1, n11, n21, b1, a2) ;
   a1 = b1 ^ c1 ;
   a2 ^= b1 & c1 ;
   ull curgen = ((nw >> 9) & 0x007f7f7f7f7f7f7fLL) +
                ((ne >> 1) & 0x0080808080808080LL) +
                ((sw << 55) & 0x7f00000000000000LL) +
                 (se << 63) ;
   return (~(a0 ^ a1)) & (a1 ^ a2) & (curgen | a1) ;
}
node *treealgo::calculate(node *nw, node *ne, node *sw, node *se, int d) {
   if (nw == 0 && ne == 0 && sw == 0 && se == 0)
      return 0 ;
   if (nw == 0)
      nw = empty ;
   if (ne == 0)
      ne = empty ;
   if (sw == 0)
      sw = empty ;
   if (se == 0)
      se = empty ;
   if (d == 4) {
      return makenode(
         calc(nw->leaf[0], nw->leaf[1], nw->leaf[2], nw->leaf[3]),
         calc(nw->leaf[1], ne->leaf[0], nw->leaf[3], ne->leaf[2]),
         calc(nw->leaf[2], nw->leaf[3], sw->leaf[0], sw->leaf[1]),
         calc(nw->leaf[3], ne->leaf[2], sw->leaf[1], se->leaf[0])) ;
   } else {
      d-- ;
      return makenode(
         calculate(nw->c[0], nw->c[1], nw->c[2], nw->c[3], d),
         calculate(nw->c[1], ne->c[0], nw->c[3], ne->c[2], d),
         calculate(nw->c[2], nw->c[3], sw->c[0], sw->c[1], d),
         calculate(nw->c[3], ne->c[2], sw->c[1], se->c[0], d)) ;
   }
}
int treealgo::nextstep(int id, int nid, int needpop) {
   if (nid != 1)
      error("! multithreading not yet supported") ;
   pushroot() ;
   node *nroot =
           calculate(root, 0, 0, 0, depth) ;
   if (nroot == 0)
      nroot = clearednode() ;
   freetree(root, depth) ;
   root = nroot ;
   poproot() ;
   if (needpop)
      return getpopulation() ;
   else
      return 0 ;
}
