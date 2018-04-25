Some code trying out different ways to evaluate Conway's
Game of Life.

Not intended for any other purpose; we are just comparing the raw
performance of basic and not-so-basic algorithms.  This means we
have butchered the golly algorithms to remove the rendering and
interactivity features.

To build, just type make.

Sample run:

   ./lifealgo -w 2048 -h 2048 -m 200 -r qlife

Algorithms that should work are:

array array2 array3 array4
avx2 avx22 avx23
bitpar bitpar2 bitpar3
hlife qlife
lifelib ulifelib
list list2 list3 list8x8 list16x16
lookup lookup2 lookup4
nybble
sse sse2 sse3
tree

Some limitations:

The following algorithm only supports even increments:

   ulifelib

Only a few algorithms support multithreading.

Results are in [results.html](http://htmlpreview.github.io/?https://github.com/rokicki/lifealg/blob/master/results.html).
