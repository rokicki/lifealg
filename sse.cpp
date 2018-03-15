#include "lifealgo.h"
#include "util.h"
#include <algorithm>
#include <cstdlib>
#include <x86intrin.h>
using namespace std ;
typedef __m128i lifeword ;
const int lifewordwidth = 128 ;
const int shift = 7 ;
class ssealgo : public lifealgo {
public:
   virtual void init(int w, int h) ;
   virtual void setcell(int x, int y) ;
   virtual int getpopulation() ;
   virtual int nextstep(int, int) ;
   virtual void swap() ;
   int w, h, wordwidth ;
   long long wh ;
   lifeword *u0, *u1 ;
} ;
static class ssealgofactory : public lifealgofactory {
public:
   ssealgofactory() ;
   virtual lifealgo *createInstance() {
      return new ssealgo() ;
   }
} factory ;
ssealgofactory::ssealgofactory() {
   registerAlgo("sse", &factory) ;
}
void ssealgo::init(int w_, int h_) {
   w = w_ ;
   h = h_ ;
   wordwidth = (w + lifewordwidth-1) >> shift ;
   wh = wordwidth * h ;
   u0 = (lifeword *)calloc(wordwidth*sizeof(lifeword), h) ;
   u1 = (lifeword *)calloc(wordwidth*sizeof(lifeword), h) ;
}
void ssealgo::setcell(int x, int y) {
   static lifeword bigbig = _mm_set_epi64x(1, 0) ;
   static lifeword bigone = _mm_cvtsi64_si128(1) ;
   int xx = x & (lifewordwidth - 1) ;
   if (xx < 64) {
      u0[(x>>shift)*h+y] |= bigone << xx ;
   } else {
      u0[(x>>shift)*h+y] |= bigbig << (xx - 64) ;
   }
}
static inline int popcount128(__m128i n) {
    const __m128i n_hi = _mm_unpackhi_epi64(n, n);
    return __builtin_popcountll(_mm_cvtsi128_si64(n)) +
           __builtin_popcountll(_mm_cvtsi128_si64(n_hi));
}
int ssealgo::getpopulation() {
   int r = 0 ;
   for (int i=0; i<wh; i++)
      r += popcount128(u0[i]) ;
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
   return _mm_slli_epi64(v, 1) | _mm_slli_si128(_mm_srli_epi64(v, 63), 8) ;
}
static lifeword rightshift(lifeword v) {
   return _mm_srli_epi64(v, 1) | _mm_srli_si128(_mm_slli_epi64(v, 63), 8) ;
}
static lifeword bigleftshift(lifeword v) {
   return _mm_slli_epi64(_mm_slli_si128(v, 8), 63) ;
}
static lifeword bigrightshift(lifeword v) {
   return _mm_srli_epi64(_mm_srli_si128(v, 8), 63) ;
}
void ssealgo::swap() { ::swap(u0, u1) ; }
int ssealgo::nextstep(int id, int n) {
   int r = 0 ;
   int loi = id * wordwidth / n ;
   int hii = (id + 1) * wordwidth / n ;
   for (int i=loi; i<hii; i++) {
      lifeword w00 = _mm_cvtsi64_si128(0) ;
      lifeword w01 = _mm_cvtsi64_si128(0) ;
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
         r += popcount128(ng1) + popcount128(ng2) ;
         w00 = w20 ;
         w01 = w21 ;
         w10 = w30 ;
         w11 = w31 ;
         w1 = w3 ;
      }
   }
   return r ;
}
