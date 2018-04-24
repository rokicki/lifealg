/*
 *   This is a super-simple aligned vector class that supports
 *   swap, [] (both left and right), allocation, destruction,
 *   push_back, and size(), and that is it.  We do not call the
 *   object destructors appropriately so it will only work on
 *   primitive types.  It is intended to give us a cheap container
 *   class for SSE types with alignment requirements.
 */
#include <cstdlib>
using namespace std ;
template<typename T, size_t gran>
class avector {
public:
   avector():a(0),cap(0),siz(0){ }
  ~avector(){ free(a) ; a = 0 ; }
   size_t size() const { return siz ; }
   const T& operator[](size_t i) { return a[i] ; }
   void clear() { siz = 0 ; }
   void push_back(const T& e) {
      if (siz+1 > cap) {
         size_t ocap = cap ;
         if (cap == 0)
            cap = 16 ;
         else
            cap = 2 * cap ;
         void *b = 0 ;
         posix_memalign(&b, gran, cap*sizeof(T)) ;
         memcpy(b, a, ocap*sizeof(T)) ;
         free(a) ;
         a = (T*)b ;
      }
      a[siz++] = e ;
   }
   void swap(avector &b) noexcept {
      std::swap(a, b.a) ; std::swap(cap, b.cap) ; std::swap(siz, b.siz) ;
   }
private:
   T *a ;
   size_t cap, siz ;
} ;
