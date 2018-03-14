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

array array2 lookup lookup2 nybble bitpar sse qlife hlife lifelib ulifelib

Some limitations:

The following algorithm only supports even increments:

   ulifelib

The following algorithms don't support multithreading:

   qlife hlife lifelib ulifelib
