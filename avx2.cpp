#ifdef __AVX2__
#include "lifealgo.h"
#include "util.h"
#include <algorithm>
#include <cstdlib>
#include <x86intrin.h>
using namespace std ;
typedef __m256i lifeword ;
const int lifewordwidth = 256 ;
const int shift = 8 ;
class avx2algo : public lifealgo {
public:
   virtual void init(int w, int h) ;
   virtual void setcell(int x, int y) ;
   virtual int getpopulation() ;
   virtual int nextstep(int, int, int) ;
   virtual void swap() ;
   int w, h, wordwidth ;
   long long wh ;
   lifeword *u0, *u1 ;
} ;
static class avx2algofactory : public lifealgofactory {
public:
   avx2algofactory() ;
   virtual lifealgo *createInstance() {
      return new avx2algo() ;
   }
} factory ;
avx2algofactory::avx2algofactory() {
   registerAlgo("avx2", &factory) ;
}
static lifeword lobit256, hibit256, notlobit256, nothibit256 ;
void avx2algo::init(int w_, int h_) {
   w = w_ ;
   h = h_ ;
   wordwidth = (w + lifewordwidth-1) >> shift ;
   wh = wordwidth * h ;
   u0 = (lifeword *)calloc(wordwidth*sizeof(lifeword), h+1) ;
   u1 = (lifeword *)calloc(wordwidth*sizeof(lifeword), h+1) ;
   union {
      lifeword w ;
      unsigned long long ll[4] ;
   } t ;
   t.ll[0] = 1 ;
   t.ll[1] = 0 ;
   t.ll[2] = 0 ;
   t.ll[3] = 0 ;
   lobit256 = t.w ;
   notlobit256 = ~t.w ;
   t.ll[0] = 0 ;
   t.ll[3] = 1LL << 63 ;
   hibit256 = t.w ;
   nothibit256 = ~t.w ;
}
void avx2algo::setcell(int x, int y) {
   union {
       lifeword w ;
       unsigned long long ll[4] ;
   } t ;
   t.ll[0] = 0 ;
   t.ll[1] = 0 ;
   t.ll[2] = 0 ;
   t.ll[3] = 0 ;
   int xx = x & (lifewordwidth - 1) ;
   t.ll[(x >> 6) & 3] |= 1LL << (xx & 63) ;
   u0[(x>>shift)*h+y] |= t.w ;
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
int avx2algo::getpopulation() {
   int r = 0 ;
   for (int i=0; i<wh; i++)
      r += popcount256(u0[i]) ;
   return r ;
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
static lifeword leftshift(lifeword v) {
   return notlobit256 & (_mm256_slli_epi64(v, 1) | _mm256_permute4x64_epi64(_mm256_srli_epi64(v, 63), 0x93)) ;
}
static lifeword rightshift(lifeword v) {
   return nothibit256 & (_mm256_srli_epi64(v, 1) | _mm256_permute4x64_epi64(_mm256_slli_epi64(v, 63), 0x39)) ;
}
static lifeword bigleftshift(lifeword v) {
   return hibit256 & _mm256_slli_epi64(_mm256_permute4x64_epi64(v, 0x39), 63) ;
}
static lifeword bigrightshift(lifeword v) {
   return lobit256 & _mm256_srli_epi64(_mm256_permute4x64_epi64(v, 0x93), 63) ;
}
static lifeword zero256 ;
void avx2algo::swap() { ::swap(u0, u1) ; }
int avx2algo::nextstep(int id, int n, int needpop) {
   int r = 0 ;
   int loi = id * wordwidth / n ;
   int hii = (id + 1) * wordwidth / n ;
   for (int i=loi; i<hii; i++) {
      lifeword w00 = zero256 ;
      lifeword w01 = zero256 ;
      lifeword *col = u0 + i * h + 1 ;
      lifeword *pcol = u0 + (i-1) * h + 1 ;
      lifeword *ncol = u0 + (i+1) * h + 1 ;
      lifeword *wcol = u1 + i * h + 1 ;
      lifeword w1 = *col ;
      lifeword w1l = leftshift(w1) ;
      lifeword w1r = rightshift(w1) ;
      if (i > 0)
         w1l += bigrightshift(*pcol) ;
      if (i+1 < wordwidth)
         w1r += bigleftshift(*ncol) ;
      lifeword w10, w11 ;
      add3(w1, w1l, w1r, w10, w11) ;
      for (int j=1; j+1<h; j += 2, col += 2, pcol += 2, ncol += 2, wcol += 2) {
         lifeword w2 = col[1] ;
         lifeword w3 = col[2] ;
         lifeword w2l = leftshift(w2) ;
         lifeword w3l = leftshift(w3) ;
         lifeword w2r = rightshift(w2) ;
         lifeword w3r = rightshift(w3) ;
         if (i > 0) {
            w2l |= bigrightshift(pcol[1]) ;
            w3l |= bigrightshift(pcol[2]) ;
         }
         if (i+1 < wordwidth) {
            w2r |= bigleftshift(ncol[1]) ;
            w3r |= bigleftshift(ncol[2]) ;
         }
         lifeword w20, w21, w30, w31, a0, a1, a2, b0, b1, b2, ng1, ng2 ;
         add3(w2, w2l, w2r, w20, w21) ;
         add2(w10, w20, b0, a1) ;
         add3(w11, w21, a1, b1, b2) ;
         add2(b0, w00, a0, a1) ;
         add3(b1, w01, a1, a1, a2) ;
         ng1 = (~(a0 ^ a1)) & (a1 ^ a2 ^ b2) & (w1 | a1) ;
         wcol[0] = ng1 ;
         add3(w3, w3l, w3r, w30, w31) ;
         add2(b0, w30, a0, a1) ;
         add3(b1, w31, a1, a1, a2) ;
         ng2 = (~(a0 ^ a1)) & (a1 ^ a2 ^ b2) & (w2 | a1) ;
         wcol[1] = ng2 ;
         if (needpop)
            r += popcount256(ng1) + popcount256(ng2) ;
         w00 = w20 ;
         w01 = w21 ;
         w10 = w30 ;
         w11 = w31 ;
         w1 = w3 ;
      }
   }
   return r ;
}
#endif
