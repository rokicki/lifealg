Some code trying out different ways to evaluate Conway's
Game of Life.

To build, just type make.

Sample run:

   ./lifealgo -w 2048 -h 2048 -m 200 -r qlife

Algorithms that should work are:

array array2 lookup lookup2 nybble bitpar sse qlife hlife lifelib

Algorithms that don't work right now:

ulifelib
