#!/usr/bin/python

from sys import argv

rulestring = argv[1]
isotrans = {}
centre = 0

for c in rulestring.lower().replace('v', 'r'):
    if c in '012345678':
        lastloc = int(c)
        isotrans[(centre, lastloc)] = "+";
    elif (c == 'b'):
        centre = 0
    elif (c == 's'):
        centre = 1
    elif (c == '/'):
        centre = 1 - centre
    elif c in 'ceaiknjqrytwz-':
        isotrans[(centre, lastloc)] += c

lord = "";
lord += "_ceaccaieaeaknja_ceaccaieaeaknjaekejanaairerririekejanaairerriri";
lord += "ccknncqnaijaqnwaccknncqnaijaqnwakykkqyqjrtjnzrqakykkqyqjrtjnzrqa";
lord += "ekirkyrtejerkkjnekirkyrtejerkkjnekejjkrnejecjyccekejjkrnejecjycc";
lord += "anriqyzraariqjqaanriqyzraariqjqajkjywkqkrnccqkncjkjywkqkrnccqknc";
lord += "cnkqccnnkqkqyykjcnkqccnnkqkqyykjaqjwinaarzjqtrnaaqjwinaarzjqtrna";
lord += "ccyyccyennkjyekeccyyccyennkjyekenykknejeirykrikenykknejeirykrike";
lord += "aqrznyirjwjqkkykaqrznyirjwjqkkykaqrqajiarqcnnkccaqrqajiarqcnnkcc";
lord += "intrneriaanajekeintrneriaanajekeajnkaeaeiaccaec_ajnkaeaeiaccaec_";
popcounts = [0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4]
lord2 = []

for i in xrange(512):

    centre = (i >> 4) & 1
    ncount = popcounts[i & 15] + popcounts[i >> 5]
    if (centre, ncount) in isotrans:
        fragment = isotrans[(centre, ncount)]
        if (len(fragment) == 1):
            lord2.append(1)
        else:
            l = 1 if (fragment[1] == '-') else 0
            for c in fragment:
                if (c == lord[i]):
                    l = 1 - l
            lord2.append(l)
    else:
        lord2.append(0)

rule_letters = {}
rule_letters[1] = "ce" ;
rule_letters[2] = "ceaikn" ;
rule_letters[3] = "ceaiknjqry" ;
rule_letters[4] = "ceaiknjqrytwz" ;
rule_letters[5] = "ceaiknjqry" ;
rule_letters[6] = "ceaikn" ;
rule_letters[7] = "ce" ;

canonicals = [['' for i in xrange(9)] for j in xrange(2)]
canstring = ''

for i in xrange(512):
    centre = (i >> 4) & 1
    ncount = popcounts[i & 15] + popcounts[i >> 5]
    if (lord2[i] == 1):
        canonicals[centre][ncount] += lord[i]

for (centre, z) in enumerate('bs'):
    canstring += z
    for ncount in xrange(9):
        goodies = set(canonicals[centre][ncount])
        if (len(goodies) == 0):
            continue
        canstring += str(ncount)
        if ncount not in rule_letters:
            continue
        a = ''.join(sorted([c for c in rule_letters[ncount] if c in goodies]))
        b = '-' + (''.join(sorted([c for c in rule_letters[ncount] if c not in goodies])))
        if (len(b) == 1):
            continue
        elif (len(b) < len(a)):
            canstring += b
        else:
            canstring += a

if (rulestring != canstring):
    print('\033[31;1mError:\033[0m %s is a non-canonical version of %s' % (rulestring, canstring))
    print('Please re-run the code with the canonical rulestring:')
    print('./recompile.sh --rule \033[36;1m%s\033[0m' % canstring)
    exit(1)

print('Compressing 512-bit lookup table for rule %s...' % rulestring)

lut9 = [sum([(lord2[8*i+j] << j) for j in xrange(8)]) for i in xrange(64)]

print('Creating magic sauce for rule %s...' % rulestring)

def oi(i0, i1, i2, i4, i5, i6):
    thisbyte = 0
    for i3 in xrange(2):
        for i7 in xrange(2):
            for i8 in xrange(2):
                thisindex = sum([(j << i) for (i, j) in enumerate([i0, i1, i2, i3, i4, i5, i6, i7, i8])])
                thisbit = 1 - lord2[thisindex]
                thisbyte |= (thisbit << (4 * i3 + 2 * i7 + i8))
    return thisbyte

def ora(i4, i5):
    return [oi(i0, i1, i2, i4, i5, i6) for i6 in xrange(2) for i1 in xrange(2) for i2 in xrange(2) for i0 in xrange(2)]

origarrays = [ora(i4, i5) for i5 in xrange(2) for i4 in xrange(2)]

maskarrays = [[0 for i in xrange(16)] for j in xrange(8)]

maskarrays[0] = [a ^ b for (a, b) in zip(origarrays[0], origarrays[2])]
maskarrays[1] = [a ^ b for (a, b) in zip(origarrays[1], origarrays[3])]
maskarrays[2] = origarrays[2]
maskarrays[3] = origarrays[3]
maskarrays[4] = [j for i in xrange(8) for j in [2 ** i, 0]]
maskarrays[5] = [j for i in xrange(8) for j in [0, 2 ** i]]
maskarrays[6] = [127] * 16
maskarrays[7] = [112] * 16

lut9 += [0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15]
lut9 += [255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0]
lut9 += [255, 255, 0, 0, 255, 255, 0, 0, 255, 255, 0, 0, 255, 255, 0, 0]

with open('lifelogic/ma_%s.h' % rulestring, 'w') as f:

    # Double every mask array for avx2 compatibility:
    marray = [x for m in maskarrays for x in (m + m)]
    f.write('// Two hundred and fifty-six seemingly arbitrary bytes\n')
    f.write('const static uint8_t __magicsauce[] __attribute__((aligned(64))) = {%d,\n' % marray[0])
    for i in xrange(15):
        currstring = '   '
        for j in xrange(i*17+1, i*17+18):
            currstring += ((' %3d};' if (j == 255) else ' %3d,') % marray[j])
        f.write(currstring + '\n')

    f.write('\n// 512-bit lookup table with extra things appended\n')
    f.write('const static uint8_t lut9[] __attribute__((aligned(64))) = {%d, %d, %d, %d,\n' % tuple(lut9[:4]))
    for i in xrange(6):
        currstring = '   '
        for j in xrange(i*18+4, i*18+22):
            currstring += ((' %3d};' if (j == 111) else ' %3d,') % lut9[j])
        f.write(currstring + '\n')

print('...completed.')
