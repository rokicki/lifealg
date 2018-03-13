#!/usr/bin/python

from sys import argv

def xprint(x):
    print ('"%s"' % x)

movups = argv[1]

if (len(argv) >= 4):
    paddb = argv[2]
    mplier = int(argv[3])
    bpr = int(argv[4])
    for i in xrange(0, 256/bpr):
        xprint(movups + " " + (str(i*bpr) if (i > 0) else '') + r"(%0), %%xmm0 \n\t")
        for _ in xrange(mplier):
            xprint(paddb  + r" %%xmm0, %%xmm0" + (", %%xmm0" if (paddb[0] == 'v') else " ") + r"\n\t")
        xprint(movups + " " + (str(i*bpr) if (i > 0) else '') + r"(%1), %%xmm1 \n\t")
        xprint(paddb  + r" %%xmm0, %%xmm1" + (", %%xmm1" if (paddb[0] == 'v') else " ") + r"\n\t")
        for _ in xrange(mplier):
            xprint(paddb  + r" %%xmm1, %%xmm1" + (", %%xmm1" if (paddb[0] == 'v') else " ") + r"\n\t")
        xprint(movups + " " + (str(i*bpr) if (i > 0) else '') + r"(%2), %%xmm0 \n\t")
        xprint(paddb  + r" %%xmm1, %%xmm0" + (", %%xmm0" if (paddb[0] == 'v') else " ") + r"\n\t")
        xprint(movups + " %%xmm0, " + (str(i*bpr) if (i > 0) else '') + r"(%3) \n\t")

elif ('add' in argv[2]):
    paddb = argv[2]
    xprint(movups + r" (%0), %%xmm0 \n\t")
    for i in xrange(1, 32):
        xprint(movups + " " + str(i*16) + r"(%0), %%xmm1 \n\t")
        xprint(paddb  + r" %%xmm1, %%xmm0" + (", %%xmm0" if (paddb[0] == 'v') else " ") + r"\n\t")
        xprint(movups + r" %%xmm0, " + str(i*16) + r"(%0) \n\t")

elif ('sub' in argv[2]):
    psubb = argv[2]
    for i in xrange(0, 16):
        xprint(movups + " " + (str(i*16) if (i > 0) else '') + r"(%0), %%xmm0 \n\t")
        xprint(movups + " " + (str(i*16) if (i > 0) else '') + r"(%1), %%xmm1 \n\t")
        xprint(psubb  + r" %%xmm0, %%xmm1" + (", %%xmm1" if (psubb[0] == 'v') else " ") + r"\n\t")
        xprint(movups + " %%xmm1, " + (str(i*16) if (i > 0) else '') + r"(%2) \n\t")
