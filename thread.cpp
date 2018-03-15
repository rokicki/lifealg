#include "thread.h"
#include <pthread.h>
const int MAX_THREADS = 128 ;
pthread_mutex_t mmutex ;
void init_mutex() {
  pthread_mutex_init(&mmutex, NULL) ;
}
void get_global_lock() {
   pthread_mutex_lock(&mmutex) ;
}
void release_global_lock() {
   pthread_mutex_unlock(&mmutex) ;
}
pthread_t p_thread[MAX_THREADS] ;
#define THREAD_RETURN_TYPE void *
#define THREAD_DECLARATOR
void spawn_thread(int i, THREAD_RETURN_TYPE(THREAD_DECLARATOR *p)(void *),
                                                                     void *o) {
   pthread_create(&(p_thread[i]), NULL, p, o) ;
}
void join_thread(int i) {
   pthread_join(p_thread[i], 0) ;
}
static int numthreads = 4 ;
void setthreadcount(int n) {
   numthreads = n ;
   init_mutex() ;
}
struct worker {
   void dowork() {
      pop = la->nextstep(i, numthreads) ;
   }
   lifealgo *la ;
   int i ;
   int pop ;
   long long pad[8] ;
} workers[MAX_THREADS] ;
void *threadworker(void *o) {
   worker *w = (worker *)o ;
   w->dowork() ;
   return 0 ;
}
int doparthreads(lifealgo *la) {
   int r = 0 ;
   for (int j=0; j<la->increment; j++) {
      for (int i=0; i<numthreads; i++) {
         workers[i].i = i ;
         workers[i].la = la ;
         spawn_thread(i, threadworker, workers+i) ;
      }
      r = 0 ;
      for (int i=0; i<numthreads; i++) {
         join_thread(i) ;
         r += workers[i].pop ;
      }
   }
   la->swap() ;
   return r ;
}
