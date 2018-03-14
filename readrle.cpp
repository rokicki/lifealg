/**
 *   Read an RLE.  We want to center it if possible, so understand
 *   a leading x = line.  Otherwise just set coordinates from the
 *   given positions.
 */
#include "lifealgo.h"
#include "util.h"
#include <cstdio>
void readrle(const char *filename, lifealgo *la, int xstart, int ystart) {
   FILE *f = fopen(filename, "r") ;
   if (f != 0)
      error("! could not open rle file") ;
   const int BUFSIZE = 8192 ;
   char inpbuf[BUFSIZE+1] ;
   int x = xstart ;
   int y = ystart ;
   int ended = 0 ;
   for (int lineno=1; fgets(inpbuf, BUFSIZE, f) != 0; lineno++) {
      if (inpbuf[0] == '#') {
         continue ;
      } else if (inpbuf[0] == 'x' && lineno == 1) {
         int patw, path ;
         if (sscanf(inpbuf, "x = %d,  y = %d", &patw, &path) != 2)
            error("! did not properly parse the x line in the RLE") ;
         xstart -= patw >> 1 ;
         ystart -= path >> 1 ;
         x = xstart ;
         y = ystart ;
      } else {
         int cnt = 0 ;
         for (char *p = inpbuf; *p; p++) {
            if (*p < ' ')
               continue ;
            if (ended)
               error("! stuff after end of RLE") ;
            if ('0' <= *p && *p <= '9') {
               cnt = 10 * cnt + *p - '0' ;
            } else {
               if (cnt == 0)
                  cnt = 1 ;
               if (*p == 'o') {
                  while (cnt-- > 0)
                     la->setcell(x++, y) ;
               } else if (*p == 'b') {
                  x += cnt ;
               } else if (*p == '$') {
                  y += cnt ;
                  x = xstart ;
               } else if (*p == '!') {
                  ended++ ;
               } else {
                  error("! unexpected char in RLE") ;
               }
               cnt = 0 ;
            }
         }
      }
   }
   fclose(f) ;
}
