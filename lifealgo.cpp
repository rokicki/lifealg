#include "lifealgo.h"
#include "thread.h"
#include "util.h"
#include <map>
#include <string>
#include <cstdlib>
#include <iostream>
#include <sys/time.h>
using namespace std ;
static double start, beginning ;
double walltime() {
   struct timeval tv ;
   gettimeofday(&tv, 0) ;
   return tv.tv_sec + 0.000001 * tv.tv_usec ;
}
double duration() {
   double now = walltime() ;
   double r = now - start ;
   start = now ;
   return r ;
}
map<string, lifealgofactory*> *factories ;
void registerAlgo(const char *name, lifealgofactory *laf) {
   if (factories == 0)
      factories = new map<string, lifealgofactory*>() ;
   (*factories)[string(name)] = laf ;
}
static int numthreads = 1 ;
int verbose = 0 ;
int main(int argc, char *argv[]) {
   int inc = 1 ;
   char *rle = 0 ;
   int maxgens = INT_MAX ;
   int random = 0 ;
   int width = 0 ;
   int height = 0 ;
   while (argc > 1 && argv[1][0] == '-') {
      argc-- ;
      argv++ ;
      switch (argv[0][1]) {
case 't':
         numthreads = atoll(argv[1]) ;
         argc-- ;
         argv++ ;
         break ;
case 'i':
         inc = atoll(argv[1]) ;
         argc-- ;
         argv++ ;
         break ;
case 'w':
         width = atoll(argv[1]) ;
         argc-- ;
         argv++ ;
         break ;
case 'h':
         height = atoll(argv[1]) ;
         argc-- ;
         argv++ ;
         break ;
case 'r':
         random++ ;
         break ;
case 'f':
         rle = argv[1] ;
         argc-- ;
         argv++ ;
         break ;
case 'm':
         maxgens = atoll(argv[1]) ;
         argc-- ;
         argv++ ;
         break ;
case 'v':
         verbose++ ;
         break ;
      }
   }
   const char *algo = argv[1] ;
   duration() ;
   if (numthreads > 1)
      setthreadcount(numthreads) ;
   if (factories->find(string(algo)) == factories->end())
      error("! no such algorithm") ;
   lifealgo *la = (*factories)[string(algo)]->createInstance() ;
   la->init(width, height) ;
   if (inc != 1)
      la->setinc(inc) ;
   if (random) {
      if (width < 3 || height < 3)
         error("! need a width and height for random") ;
      for (int y=1; y+1<height; y++)
         for (int x=1; x+1<width; x++)
            if (drand48() < 0.37)
               la->setcell(x, y) ;
   } else if (rle) {
      readrle(rle, la, width>>1, height>>1) ;
   } else {
      error("! either give -r or -f rlefile") ;
   }
   int pop = la->getpopulation() ;
   cout << "Initialized in " << duration() << " population = " << pop << endl ;
   for (int g=0; g < maxgens; g += inc) {
      if (numthreads <= 1)
         pop = la->nextstep() ;
      else
         pop = doparthreads(la) ;
      if (verbose)
         cout << (g+inc) << " " << pop << endl << flush ;
   }
   double tim = duration() ;
   double cgps = width * (double)height * (double)maxgens / tim ;
   cout << "Final pop is " << pop << " time " << tim << " cgps " << cgps << endl ;
   if (la->getpopulation() != pop) {
      cout << "Execution returned " << pop << " but scan says " <<
              la->getpopulation() << endl ;
      error("! population mismatch") ;
   }
}
