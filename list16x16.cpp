#include "lifealgo.h"
#include <algorithm>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include "avector.h"
#include <x86intrin.h>
using namespace std ;
typedef unsigned long long ull ;
typedef long long ll ;
typedef __m256i lifeword ;
const ll END = 0x7ffffffffffffff0LL ;
class list16x16algo : public lifealgo {
public:
   virtual void init(int w, int h) ;
   virtual void setcell(int x, int y) ;
   virtual int getpopulation() ;
   virtual int nextstep(int, int, int) ;
   virtual void swap() ;
   void createit() ;
   void flushit(vector<pair<pair<ll, ll>, lifeword> > &trow) ;
   int created ;
   vector<pair<int, int> > pts ;
   vector<ll> a0, a1 ;
   avector<lifeword, sizeof(lifeword)> lw0, lw1 ;
} ;
static class list16x16algofactory : public lifealgofactory {
public:
   list16x16algofactory() ;
   virtual lifealgo *createInstance() {
      return new list16x16algo() ;
   }
} factory ;
list16x16algofactory::list16x16algofactory() {
   registerAlgo("list16x16", &factory) ;
}
void list16x16algo::init(int w_, int h_) {
}
void list16x16algo::setcell(int x, int y) {
   created = 0 ;
   pts.push_back(make_pair(y, x)) ;
}
static inline int popcount256(lifeword n) {
    union {
       lifeword w ;
       unsigned long long ll[4] ;
    } t ;
    t.w = n ;
    return __builtin_popcountll(t.ll[0]) +
           __builtin_popcountll(t.ll[1]) +
           __builtin_popcountll(t.ll[2]) +
           __builtin_popcountll(t.ll[3]) ;
}
static inline int iszero(lifeword n) {
   return _mm256_testz_si256(n, n) == 1 ;
}
int list16x16algo::getpopulation() {
   if (!created)
      return pts.size() ;
   int r = 0 ;
   for (size_t i=0; i<lw0.size(); i++)
      r += popcount256(lw0[i]) ;
   return r ;
}
void list16x16algo::swap() { }
static lifeword zero, singlebits[256] ;
bool onlyfirst(const pair<pair<ll, ll>, lifeword> &a,
               const pair<pair<ll, ll>, lifeword> &b) {
   return a.first < b.first ;
}
void list16x16algo::flushit(vector<pair<pair<ll, ll>, lifeword> > &trow) {
   if (trow.size() == 0)
      return ;
   sort(trow.begin(), trow.end(), onlyfirst) ;
   a0.push_back(trow[0].first.first << 4) ;
   for (size_t i=0; i<trow.size(); ) {
      lifeword bits = trow[i].second ;
      size_t j = 1 ;
      while (i+j < trow.size() && trow[i].first == trow[i+j].first) {
         bits |= trow[i+j].second ;
         j++ ;
      }
      a0.push_back(trow[i].first.second << 4) ;
      lw0.push_back(bits) ;
      i += j ;
   }
   a0.push_back(END) ;
   trow.clear() ;
}
void list16x16algo::createit() {
   if (!created) {
      if (iszero(singlebits[0]))
         for (int i=0; i<256; i++) {
            union {
               lifeword w ;
               unsigned long long ll[4] ;
            } t ;
            t.ll[0] = t.ll[1] = t.ll[2] = t.ll[3] = 0 ;
            t.ll[i>>6] |= 1LL << (i & 63) ;
            singlebits[i] = t.w ;
         }
      sort(pts.begin(), pts.end()) ;
      ll cury = END ;
      vector<pair<pair<ll, ll>, lifeword> > trow ;
      for (size_t i=0; i<pts.size(); i++) {
         ll yy = pts[i].first >> 4 ;
         if (yy != cury) {
            flushit(trow) ;
            cury = yy ;
         }
         trow.push_back(make_pair(make_pair(yy, pts[i].second >> 4),
            singlebits[(pts[i].second & 15) + 16 * (pts[i].first & 15)])) ;
      }
      flushit(trow) ;
      if (cury != END)
         a0.push_back(END) ;
      created = 1 ;
   }
   // free the storage associated with the set cells
   vector<pair<int, int> > t ;
   ::swap(t, pts) ;
}
static inline void add2(lifeword a, lifeword b,
                        lifeword &c0, lifeword &c1) {
   c0 = a ^ b ;
   c1 = a & b ;
}
static inline void add3(lifeword a, lifeword b, lifeword c,
                        lifeword &c0, lifeword &c1) {
   lifeword t0, t1, t2 ;
   add2(a, b, t0, t1) ;
   add2(t0, c, c0, t2) ;
   c1 = t2 | t1 ;
}
/*
 *   Calculate the 16x16 universe one down and right from the 32x32 universe
 *   in the four provided cells; each is an 16x16 universe.  We use bit tricks.
 */
static lifeword calc(lifeword nw, lifeword ne, lifeword sw, lifeword se) {
   lifeword w0, w1, e0, e1, a0, a1, a2, c1, b1 ;
   lifeword nw0 = _mm256_alignr_epi8(_mm256_permute2x128_si256(nw,sw,0x21),nw,2) ;
   lifeword nw1 = _mm256_alignr_epi8(_mm256_permute2x128_si256(nw,sw,0x21),nw,4) ;
   lifeword ne0 = _mm256_alignr_epi8(_mm256_permute2x128_si256(ne,se,0x21),ne,2) ;
   lifeword ne1 = _mm256_alignr_epi8(_mm256_permute2x128_si256(ne,se,0x21),ne,4) ;
   lifeword curgen = _mm256_srli_epi16(nw0, 1) | _mm256_slli_epi16(ne0, 15) ;
   add3(nw, nw0, nw1, w0, w1) ;
   add3(ne, ne0, ne1, e0, e1) ;
   lifeword n10 = _mm256_srli_epi16(w0, 1) | _mm256_slli_epi16(e0, 15) ;
   lifeword n11 = _mm256_srli_epi16(w1, 1) | _mm256_slli_epi16(e1, 15) ;
   lifeword n20 = _mm256_srli_epi16(w0, 2) | _mm256_slli_epi16(e0, 14) ;
   lifeword n21 = _mm256_srli_epi16(w1, 2) | _mm256_slli_epi16(e1, 14) ;
   add3(w0, n10, n20, a0, c1) ;
   add3(w1, n11, n21, b1, a2) ;
   a1 = b1 ^ c1 ;
   a2 ^= b1 & c1 ;
   return (a0 ^ a2) & (a1 ^ a2) & (curgen | a1) ;
}
int list16x16algo::nextstep(int id, int nid, int needpop) {
   int r = 0 ;
   if (nid != 1)
      error("! multithreading not yet supported") ;
   if (!created)
      createit() ;
   a1.clear() ;
   lw1.clear() ;
   ll r0=0, r1=0 ;
   ll wy = END ;
   ll lwp0 = 0 ;
   ll lwp1 = 0 ;
   while (a0[r0] != END) {
      ll y = min(a0[r0], a0[r1]-16) ;
      ll p0 = r0+1 ;
      ll p1 = r1 ;
      if (a0[p1] != END)
         p1++ ;
      ll x = -END ;
      while (1) {
         ll xt = END ;
         if (a0[r0] == y)
            xt = a0[p0] ;
         if (a0[r1] == y+16)
            xt = min(xt, a0[p1]) ;
         if (xt == END) {
            if (a0[r0] == y)
               r0 = p0 + 1 ;
            if (a0[r1] == y+16)
               r1 = p1 + 1 ;
            break ;
         }
         x = max(x, xt-16) ;
         lifeword nw, ne, sw, se ;
         if (a0[r0] == y) {
            if (a0[p0] == x) {
               nw = lw0[lwp0] ;
               p0++ ;
               lwp0++ ;
            } else
               nw = zero ;
            if (a0[p0] == x+16)
               ne = lw0[lwp0] ;
            else
               ne = zero ;
         } else {
            nw = zero ;
            ne = zero ;
         }
         if (a0[r1] == y+16) {
            if (a0[p1] == x) {
               sw = lw0[lwp1] ;
               p1++ ;
               lwp1++ ;
            } else
               sw = zero ;
            if (a0[p1] == x+16)
               se = lw0[lwp1] ;
            else
               se = zero ;
         } else {
            sw = zero ;
            se = zero ;
         }
         lifeword nb = calc(nw, ne, sw, se) ;
         if (!iszero(nb)) {
            if (wy != y+1) {
               if (wy != END)
                  a1.push_back(END) ;
               wy = y+1 ;
               a1.push_back(y+1) ;
            }
            a1.push_back(x+1) ;
            lw1.push_back(nb) ;
            if (needpop)
               r += popcount256(nb) ;
         }
         x += 16 ;
      }
   }
   a1.push_back(END) ;
   if (wy != END)
      a1.push_back(END) ;
   ::swap(a0, a1) ;
   lw0.swap(lw1) ;
   return r ;
}
