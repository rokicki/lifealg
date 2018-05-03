#ifndef JUST32BIT
#include "lifealgo.h"
#include "util.h"
#include <algorithm>
#include <cstdlib>
#include <x86intrin.h>
using namespace std ;
typedef __m128i lifeword ;
const int lifewordwidth = 128 ;
const int shift = 7 ;
class sse3algo : public lifealgo {
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
static class sse3algofactory : public lifealgofactory {
public:
   sse3algofactory() ;
   virtual lifealgo *createInstance() {
      return new sse3algo() ;
   }
} factory ;
sse3algofactory::sse3algofactory() {
   registerAlgo("sse3", &factory) ;
}
void sse3algo::init(int w_, int h_) {
   w = w_ ;
   h = h_ ;
   wordwidth = (w + lifewordwidth-1) >> shift ;
   wh = wordwidth * h ;
   u0 = (lifeword *)calloc(wordwidth*sizeof(lifeword), h+1) ;
   u1 = (lifeword *)calloc(wordwidth*sizeof(lifeword), h+1) ;
}
void sse3algo::setcell(int x, int y) {
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
int sse3algo::getpopulation() {
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
void sse3algo::swap() { ::swap(u0, u1) ; }
int sse3algo::nextstep(int id, int n, int needpop) {
   int r = 0 ;
   int loi = id * wordwidth / n ;
   int hii = (id + 1) * wordwidth / n ;
   for (int i=loi; i<hii; i++) {
      lifeword w300 = _mm_cvtsi64_si128(0) ;
      lifeword w301 = _mm_cvtsi64_si128(0) ;
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
      lifeword w210, w211, w310, w311 ;
      add2(w1l, w1r, w210, w211) ;
      add2(w1, w210, w310, w311) ;
      w311 |= w211 ;
#pragma unroll 8
      for (int j=1; j+1<h; j++, col++, pcol++, ncol++, wcol++) {
         lifeword w2 = col[1] ;
         lifeword w2l = leftshift(w2) ;
         lifeword w2r = rightshift(w2) ;
         if (i > 0)
            w2l |= bigrightshift(pcol[1]) ;
         if (i+1 < wordwidth)
            w2r |= bigleftshift(ncol[1]) ;
         lifeword w220, w221, w320, w321, a0, a1 ;
         add2(w2l, w2r, w220, w221) ;
         add2(w2, w220, w320, w321) ;
         w321 |= w221 ;
         add3(w300, w210, w320, a0, a1) ;
         lifeword ng1 = (a1 ^ w301 ^ w211 ^ w321) &
                                   ((a1 | w301) ^ (w211 | w321)) & (a0 | w1) ;
         wcol[0] = ng1 ;
         if (needpop)
            r += popcount128(ng1) ;
         w300 = w310 ;
         w301 = w311 ;
         w310 = w320 ;
         w311 = w321 ;
         w210 = w220 ;
         w211 = w221 ;
         w1 = w2 ;
      }
   }
   return r ;
}
#endif
