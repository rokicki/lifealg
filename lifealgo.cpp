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
   while (argc > 1 && argv[1][0] == '-') {
      argc-- ;
      argv++ ;
      switch (argv[0][1]) {
case 't':
         numthreads = atoll(argv[1]) ;
         argc-- ;
         argv++ ;
         break ;
case 'v':
         verbose++ ;
         break ;
      }
   }
   const char *algo = argv[1] ;
   int sz = atoll(argv[2]) ;
   int gens = atoll(argv[3]) ;
   duration() ;
   if (numthreads > 1)
      setthreadcount(numthreads) ;
   if (factories->find(string(algo)) == factories->end())
      error("! no such algorithm") ;
   lifealgo *la = (*factories)[string(algo)]->createInstance() ;
   la->init(sz, sz) ;
   for (int y=1; y+1<sz; y++)
      for (int x=1; x+1<sz; x++)
         if (drand48() < 0.288)
            la->setcell(x, y) ;
   int pop = la->getpopulation() ;
   cout << "Initialized in " << duration() << " population = " << pop << endl ;
   for (int g=1; g <= gens; g++) {
      if (numthreads <= 1)
         pop = la->nextstep() ;
      else
         pop = doparthreads(la) ;
      if (verbose)
         cout << g << " " << pop << endl << flush ;
   }
   double tim = duration() ;
   double cgps = sz * (double)sz * (double)gens / tim ;
   cout << "Final pop is " << pop << " time " << tim << " cgps " << cgps << endl ;
   if (la->getpopulation() != pop) {
      cout << "Execution returned " << pop << " but scan says " <<
              la->getpopulation() << endl ;
      error("! population mismatch") ;
   }
}
