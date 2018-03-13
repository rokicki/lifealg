#!/usr/bin/python

# rule2asm.py updated for lifelib

import os
import re
import sys

def get_script_path():
    return os.path.dirname(os.path.realpath(sys.argv[0]))

class iwriter:

    def printinstr(self, s):

        self.f.write('        "%s \\n\\t"\n' % s)

    def printcomment(self, s):

        self.f.write('        // %s \n' % s)

    def preparethings(self, dwidth, reg13=13, reg14=14):

        regname = '%%ymm' if ('avx2' in self.iset) else '%%xmm'
        r13 = regname + str(reg13)
        r14 = regname + str(reg14)

        if ('avx2' in self.iset):
            self.printinstr('vmovdqu (%2), '+r14)
            self.printinstr('vmovdqu 32(%2), '+r13)
        else:
            shuf = 'vpshufd' if ('avx' in self.iset) else 'pshufd'
            self.printinstr('mov $0xffffffff, %%ebx')
            self.printinstr('movd %%ebx, ' + r13)
            if (dwidth):
                ddic = {28: '0x3ffffffc',
                        24: '0x0ffffff0',
                        20: '0x03ffffc0',
                        16: '0x00ffff00',
                        12: '0x003ffc00',
                         8: '0x000ff000',
                         4: '0x0003c000'}
                self.printinstr('mov $' + ddic[dwidth] + ', %%ebx')
                self.printinstr('movd %%ebx, ' + r14)
            self.printinstr('%s $1, %s, %s' % (shuf, r13, r13))
            self.printinstr('%s $0, %s, %s' % (shuf, r14, r14))

    def logicgate(self, op, inreg1, inreg2, outreg, regname=None):

        if regname is None:
            regname = '%%ymm' if ('avx2' in self.iset) else '%%xmm'
        i1 = regname + str(inreg1)
        i2 = regname + str(inreg2)
        o1 = regname + str(outreg)

        if 'avx' in self.iset:
            self.printinstr('v%s %s, %s, %s' % (op, i1, i2, o1))
        elif (i2 == o1):
            self.printinstr('%s %s, %s' % (op, i1, o1))
        elif (i1 == o1):
            if (op == 'pandn'):
                self.printinstr('por %s, %s' % (i2, o1))
                self.printinstr('pxor %s, %s' % (i2, o1))
            else:
                self.printinstr('%s %s, %s' % (op, i2, o1))
        else:
            self.printinstr('movdqa %s, %s' % (i2, o1))
            self.printinstr('%s %s, %s' % (op, i1, o1))

    def ntinit(self):
        '''
        Prepare lookup tables, masks, et cetera:
        '''

        regbytes = 32 if ('avx2' in self.iset) else 16
        regname = '%%ymm' if ('avx2' in self.iset) else '%%xmm'
        accessor = 'vmovdqu' if ('avx' in self.iset) else 'movups'
        for i in xrange(8):
            memloc = '(%2)' if (i == 0) else (str(32*i) + '(%2)')
            self.printinstr('%s %s, %s' % (accessor, memloc, regname + str(i + 2)))
        self.logicgate('pxor', 14, 14, 14)

    def ntiter(self):
        '''
        Evaluates 16 or 32 cell-updates in 16 machine instructions (excluding
        additional movdqa instructions emitted for SSE machines).

        Assumes the following initial states:
                  xmm0: bytes containing neighbour states (0..255);
                  xmm1: bytes containing centre states (0..1);
            xmm2..xmm7: lookup tables for pshufb;
                  xmm8: uniform array containing 0x7f;
                  xmm9: uniform array containing 0x70;
                 xmm14: uniform array containing 0x00.

        The output will be stored in xmm10. Register xmm0 is clobbered.
        '''

        self.logicgate('pshufb', 0, 2, 10)
        self.logicgate('pshufb', 0, 3, 11)
        self.logicgate('pand', 8, 0, 0)
        self.logicgate('pshufb', 0, 4, 12)
        self.logicgate('pshufb', 0, 5, 13)
        self.logicgate('pxor', 12, 10, 10)
        self.logicgate('pxor', 13, 11, 11)
        self.logicgate('pand', 9, 0, 0)

        regname = '%%ymm' if ('avx2' in self.iset) else '%%xmm'
        if ('avx' in self.iset):
            self.printinstr('vpsrld $3, %s0, %s0' % (regname, regname))
        else:
            self.printinstr('psrld $3, %s0' % regname)

        self.logicgate('por', 1, 0, 0)
        self.logicgate('pshufb', 0, 6, 12)
        self.logicgate('pshufb', 0, 7, 13)
        self.logicgate('pand', 12, 10, 10)
        self.logicgate('pand', 13, 11, 11)
        self.logicgate('por', 11, 10, 10)
        self.logicgate('pcmpeqb', 14, 10, 10)

    def load_and_hshift(self, i, oddgen, terminal):

        regbytes = 32 if ('avx2' in self.iset) else 16
        regname = '%%ymm' if (('avx2' in self.iset) and not terminal) else '%%xmm'
        accessor = 'vmovdqu' if ('avx' in self.iset) else 'movups'
        inreg = regname + str(5 - 3 * (i % 2))

        d = '(%1)' if (oddgen) else '(%0)'
        d = d if (i == 0) else (str(regbytes * i) + d)

        self.printinstr('%s %s, %s' % (accessor, d, inreg))
        if ('avx' in self.iset):
            self.printinstr('vpsrld $1, %s, %s0' % (inreg, regname))
            self.printinstr('vpslld $1, %s, %s1' % (inreg, regname))
        else:
            self.printinstr('movdqa %s, %s0' % (inreg, regname))
            self.printinstr('movdqa %s, %s1' % (inreg, regname))
            self.printinstr('psrld $1, %s0' % regname)
            self.printinstr('pslld $1, %s1' % regname)

    def horizontal_adders(self, i):

        self.logicgate('pxor', 0, 1, 6 - 3 * (i % 2))
        self.logicgate('pand', 0, 1, 7 - 3 * (i % 2))
        self.logicgate('pand', 5 - 3 * (i % 2), 6 - 3 * (i % 2), 1)
        self.logicgate('pxor', 5 - 3 * (i % 2), 6 - 3 * (i % 2), 6 - 3 * (i % 2))
        self.logicgate('por', 1, 7 - 3 * (i % 2), 7 - 3 * (i % 2))

    def vertical_bitshifts(self, i):

        if 'avx2' in self.iset:
            self.logicgate('pblendd $1,', 6 - 3 * (i % 2), 3 + 3 * (i % 2), 8)
            self.logicgate('pblendd $1,', 7 - 3 * (i % 2), 4 + 3 * (i % 2), 9)
            self.logicgate('pblendd $3,', 6 - 3 * (i % 2), 3 + 3 * (i % 2), 10)
            self.logicgate('pblendd $3,', 7 - 3 * (i % 2), 4 + 3 * (i % 2), 11)
            self.logicgate('pblendd $1,', 5 - 3 * (i % 2), 2 + 3 * (i % 2), 12)
            self.logicgate('permd', 8, 13, 8)
            self.logicgate('permd', 9, 13, 9)
            self.printinstr('vpermq $57, %%ymm10, %%ymm10')
            self.printinstr('vpermq $57, %%ymm11, %%ymm11')
            self.logicgate('permd', 12, 13, 12)
        else:
            self.logicgate('pand', 13, 3 + 3 * (i % 2), 8)
            self.logicgate('pand', 13, 4 + 3 * (i % 2), 9)
            self.logicgate('pand', 13, 2 + 3 * (i % 2), 12)
            self.logicgate('pandn', 6 - 3 * (i % 2), 13, 0)
            self.logicgate('por', 0, 8, 8)
            self.logicgate('pandn', 7 - 3 * (i % 2), 13, 0)
            self.logicgate('por', 0, 9, 9)
            self.logicgate('pandn', 5 - 3 * (i % 2), 13, 0)
            self.logicgate('por', 0, 12, 12)
            self.logicgate('shufps $0x39,', 8, 8, 8)
            self.logicgate('shufps $0x39,', 9, 9, 9)
            self.logicgate('shufps $0x4e,', 6 - 3 * (i % 2), 3 + 3 * (i % 2), 10)
            self.logicgate('shufps $0x4e,', 7 - 3 * (i % 2), 4 + 3 * (i % 2), 11)
            self.logicgate('shufps $0x39,', 12, 12, 12)

    def vertical_adders(self, i):

        self.logicgate('pxor', 3 + 3 * (i % 2), 8, 8)
        self.logicgate('pxor', 4 + 3 * (i % 2), 9, 9)
        self.logicgate('pxor', 8, 10, 10)
        self.logicgate('pxor', 9, 11, 11)
        self.logicgate('por', 8, 3 + 3 * (i % 2), 3 + 3 * (i % 2))
        self.logicgate('por', 9, 4 + 3 * (i % 2), 4 + 3 * (i % 2))
        self.logicgate('pand', 10, 8, 8)
        self.logicgate('pand', 11, 9, 9)
        self.logicgate('pandn', 3 + 3 * (i % 2), 8, 8)
        self.logicgate('pandn', 4 + 3 * (i % 2), 9, 9)

    def genlogic(self, rulestring):

        bee = [0] * 10
        ess = [0] * 10

        for char in rulestring:
            if (char == 'b'):
                birth = True
            elif (char == 's'):
                birth = False
            else:
                k = int(char)
                if (birth):
                    bee[k] = 1
                else:
                    ess[k+1] = 1

        negate = bee[0]
        beexor = (bee[0] != bee[8])
        essxor = (ess[1] != ess[9])

        stars = ("*" if essxor else "") + ("**" if beexor else "") + ("!" if negate else "")

        if negate:
            bee = [1 - x for x in bee]
            ess = [1 - x for x in ess]

        ess[0] = (1 - ess[8]) if essxor else ess[8]
        usetopbit = (essxor or beexor)

        self.logicgate('pand', 8, 11, 1)
        self.logicgate('pxor', 11, 8, 8)
        if (beexor and not essxor):
            self.logicgate('pand', 1, 9, 0)
            self.logicgate('pandn', 0, 12, 11)
        elif (usetopbit):
            self.logicgate('pand', 1, 9, 11)
        if (essxor and not beexor):
            self.logicgate('pand', 12, 11, 11)
        self.logicgate('pxor', 1, 9, 9)

        ruleint = 0;
        for i in xrange(8):
            ruleint += (bee[i] << i)
            ruleint += (ess[i] << (i + 8))

        print("Rule integer:     "+str(ruleint)+stars)
        regnames = [10, 8, 9, 12, 1, 0]
        opnames = ["and", "or", "andn", "nonsense", "xor"]
        rident = None

        with open('boolean.out', 'r') as f:
            for fline in f:
                x = fline.split(":  ")
                if (len(x) == 2) and (int(x[0]) == ruleint):
                    rident = x[1][:-1]

        if (ruleint == 0):
            rident = '-004'
        if (ruleint == 65280):
            rident = '-331'
        if (ruleint == 61680):
            rident = '-221'
        if (ruleint == 52428):
            rident = '-111'
        if (ruleint == 43690):
            rident = '-001'

        if rident is None:
            print("Error: unrecognised rule")
            exit(1)
        else:
            print("Rule circuit:     ["+rident+"]"+stars)

        rchars = list(rident)
        for i in xrange(len(rchars)):
            if (rchars[i] == '-'):
                rchars[i] = (i/4) + 4
            else:
                rchars[i] = int(rchars[i])

        for i in xrange(0, len(rchars), 4):
            if (rchars[i+3] == 3):
                rchars[i+3] = 2
                rchars[i+2] ^= rchars[i+1]
                rchars[i+1] ^= rchars[i+2]
                rchars[i+2] ^= rchars[i+1]

        # Map logical registers to physical registers:
        for i in xrange(0, len(rchars), 4):
            dependencies = [False] * 10
            for j in xrange(i+4, len(rchars), 4):
                dependencies[rchars[j+1]] = True
                dependencies[rchars[j+2]] = True
            d = -1
            e = rchars[i]
            for j in xrange(6):
                if (e == j):
                    d = j
                    break
                if not dependencies[j]:
                    d = j
                    break
            if (d == -1):
                print("Error: insufficiently many physical registers")
                exit(1)
            else:
                # print(str(e)+" --> "+str(d))
                rchars[i] = d
                for j in xrange(i+4, len(rchars), 4):
                    if (rchars[j+1] == e):
                        rchars[j+1] = d
                    if (rchars[j+2] == e):
                        rchars[j+2] = d

        if (rulestring == 'b3s23'):
            # Possibly more optimal due to instruction order:
            self.logicgate('pxor', 9, 8, 8)
            self.logicgate('por', 10, 12, 12)
            self.logicgate('pxor', 9, 10, 10)
            self.logicgate('pand', 12, 8, 8)
            self.logicgate('pand', 8, 10, 10)
        else:
            for i in xrange(0, len(rchars), 4):
                self.logicgate('p'+opnames[rchars[i+3]], regnames[rchars[i+1]], regnames[rchars[i+2]], regnames[rchars[i]])

        if (usetopbit):
            # printcomment(g, 'correct for B8/S8 nonsense:')
            self.logicgate('pxor', 11, 10, 10)

        if (negate):
            # Rule contains B0:
            self.logicgate('pcmpeqb', 11, 11, 11)
            self.logicgate('pxor', 11, 10, 10)


    def save_result(self, i, oddgen, terminal, diff=False):

        regbytes = 32 if ('avx2' in self.iset) else 16
        if oddgen:
            e = str(regbytes * (i - 1) + 8) + '(%0)'
        else:
            e = '(%1)' if (i == 1) else (str(regbytes * (i - 1)) + '(%1)')

        regname = '%%ymm' if (('avx2' in self.iset) and not terminal) else '%%xmm'
        accessor = 'vmovdqu' if ('avx' in self.iset) else 'movups'
        if diff:
            self.logicgate('pand', 14, 10, 10, regname)
            self.printinstr('%s %s, %s8' % (accessor, e, regname))
            self.logicgate('pandn', 8, 14, 11, regname)
            self.logicgate('por', 10, 11, 11, regname)
            self.printinstr('%s %s11, %s' % (accessor, regname, e))
            regname = '%%ymm' if ('avx2' in self.iset) else '%%xmm'
            if (diff == 'initial'):
                self.logicgate('pxor', 11, 8, 15)
                self.printinstr('%s %s15, %s' % (accessor, regname, '(%1)'))
            else:
                self.logicgate('pxor', 11, 8, 8)
                self.logicgate('por', 8, 15, 15)
            if (diff == 'final'):
                pos2 = '64(%1)' if ('avx2' in self.iset) else '32(%1)'
                pos1 = '32(%1)' if ('avx2' in self.iset) else '16(%1)'
                self.printinstr('%s %s8, %s' % (accessor, regname, pos2))
                self.printinstr('%s %s15, %s' % (accessor, regname, pos1))
        else:
            self.printinstr('%s %s10, %s' % (accessor, regname, e))

    def prologue(self):

        self.f.write('        asm (\n')

    def epilogue(self, dwidth):

        self.f.write('                : /* no output operands */ \n')
        self.f.write('                : "r" (d), "r" (e)')
        if (dwidth):
            self.f.write(', "r" (apg::__sixteen%d)' % dwidth)
        self.f.write('\n')
        self.f.write('                : "ebx", ')
        for i in xrange(16):
            self.f.write('"xmm%d", ' % i)
            if (i % 6 == 4):
                self.f.write('\n' + (' ' * 20))
        self.f.write('"memory");\n\n')

    def assemble(self, rulestring, oddgen, rowcount, dwidth):

        self.prologue()
        self.preparethings(dwidth)

        if ('avx2' in self.iset):
            iters = rowcount / 8 + (2 if ((rowcount % 8) and not oddgen) else 1)
            rpr = 8
        else:
            iters = rowcount / 4 + (0 if oddgen else 1)
            rpr = 4

        for i in xrange(iters):
            if (i * rpr < rowcount):
                terminal = ((i + 1) * rpr > rowcount)
                self.load_and_hshift(i, oddgen, terminal)
                self.horizontal_adders(i)
            if (i > 0):
                self.vertical_bitshifts(i)
                self.vertical_adders(i)
                self.f.write('#include "ll_%s_%s.asm"\n' % (self.besti, rulestring))
                terminal = (i * rpr == rowcount + (0 if oddgen else 4))
                if oddgen:
                    if (i == 1):
                        diff = 'initial'
                    elif (i + 1 == iters):
                        diff = 'final'
                    else:
                        diff = True
                else:
                    diff = False
                self.save_result(i, oddgen, terminal, diff)

        self.epilogue(dwidth)

    def gwrite_function(self, rulestring, rowcount, dwidth):

        name = 'iterate_%s_%d_%d' % (self.besti, rowcount, dwidth)
        params = 'uint32_t * __restrict__ d, uint32_t * __restrict__ e, uint32_t * __restrict__ h, uint32_t * __restrict__ j'
        self.f.write('    void %s(%s) {\n' % (name, params))
        logstring = rulestring[rulestring.index('b'):]
        self.assemble(logstring, 0, rowcount, dwidth)
        self.f.write('            for (int i = 1; i < %d; i++) {\n' % (rowcount - 1))
        self.f.write('                e[i-1] &= (~h[i]);\n')
        self.f.write('                j[i] = d[i] & (~e[i-1]);\n')
        self.f.write('                d[i] = e[i-1];\n')
        self.f.write('            }\n')
        self.f.write('        return;\n')
        self.f.write('    }\n\n')

    def write_function(self, rulestring, rowcount, dwidth):

        name = 'iterate_%s_%d_%d' % (self.besti, rowcount, dwidth)

        params = 'uint32_t * __restrict__ diffs, bool onegen'
        for i in 'jhed':
            params = 'uint32_t * __restrict__ ' + i + ', ' + params

        self.f.write('    bool %s(%s) {\n' % (name, params))

        self.f.write('        if (h) {\n')
        self.f.write('            for (int i = 0; i < %d; i++) {\n' % (rowcount))
        self.f.write('                h[i] |= d[i];\n')
        self.f.write('            }\n')
        self.f.write('        }\n')

        self.f.write('        if (j) {\n')
        self.f.write('            for (int i = 0; i < %d; i++) {\n' % (rowcount))
        self.f.write('                j[i] &= d[i];\n')
        self.f.write('            }\n')
        self.f.write('        }\n')

        self.assemble(rulestring, 0, rowcount, dwidth)

        self.f.write('        if (h) {\n')
        self.f.write('            for (int i = 1; i < %d; i++) {\n' % (rowcount - 1))
        if (rulestring[:2] == 'b0'):
            # We want the history state to match the envelope of the Gollyfied
            # version of the B0 rule:
            self.f.write('                h[i] |= (~e[i-1]);\n')
        else:
            self.f.write('                h[i] |= e[i-1];\n')
        self.f.write('            }\n')
        self.f.write('        }\n')

        self.f.write('        if (j) {\n')
        self.f.write('            for (int i = 1; i < %d; i++) {\n' % (rowcount - 1))
        self.f.write('                j[i] &= e[i-1];\n')
        self.f.write('            }\n')
        self.f.write('        }\n')

        self.f.write('        if (onegen) {\n')
        self.f.write('            for (int i = 2; i < %d; i++) {\n' % (rowcount - 2))
        self.f.write('                d[i] = e[i-1];\n')
        self.f.write('            }\n')
        self.f.write('            return false;\n')
        self.f.write('        }\n')

        self.assemble(rulestring, 1, rowcount, dwidth)

        self.f.write('        if (h) {\n')
        self.f.write('            for (int i = 2; i < %d; i++) {\n' % (rowcount - 2))
        self.f.write('                h[i] |= d[i];\n')
        self.f.write('            }\n')
        self.f.write('        }\n')

        self.f.write('        if (j) {\n')
        self.f.write('            for (int i = 2; i < %d; i++) {\n' % (rowcount - 2))
        self.f.write('                j[i] &= d[i];\n')
        self.f.write('            }\n')
        self.f.write('        }\n')

        if 'avx2' in self.iset:
            self.f.write('        uint32_t bigdiff = e[8] | e[9] | e[10] | e[11] | e[12] | e[13] | e[14] | e[15];\n')
            self.f.write('        if (diffs != 0) {\n')
            self.f.write('        diffs[0] = bigdiff;\n')
            self.f.write('        diffs[1] = e[0] | e[1];\n')
            self.f.write('        diffs[2] = e[18] | e[19];\n')
            self.f.write('        }\n')
        else:
            self.f.write('        uint32_t bigdiff = e[4] | e[5] | e[6] | e[7];\n')
            self.f.write('        if (diffs != 0) {\n')
            self.f.write('        diffs[0] = bigdiff;\n')
            self.f.write('        diffs[1] = e[0] | e[1];\n')
            self.f.write('        diffs[2] = e[10] | e[11];\n')
            self.f.write('        }\n')

        self.f.write('        return (bigdiff == 0);\n')
        self.f.write('    }\n\n')

    def gwrite_iterator(self):

        name = 'iterate_var_%s' % self.besti
        self.f.write('    void %s(uint32_t * __restrict__ d, uint32_t * __restrict__ h) {\n' % name)
        self.f.write('        uint32_t e[32];\n')
        self.f.write('        uint32_t j[32];\n')
        self.f.write('        iterate_%s_20_16(d+6, e+6, h+6, j+6);\n' % self.besti)
        self.f.write('        for (int i = 8; i < 24; i++) { h[i] = j[i]; }\n')
        self.f.write('        return;\n')
        self.f.write('    }\n\n')

    def write_iterator(self):

        name = 'iterate_var_%s' % self.besti
        self.f.write('    int %s(int n, uint32_t * __restrict__ d, uint32_t * __restrict__ h, uint32_t * __restrict__ j) {\n' % name)
        self.f.write('        uint32_t e[32];\n')
        self.f.write('        if (n >= 7) { if (iterate_%s_32_28(d, e, h, j, 0, (n == 7))) {return 8;} }\n' % self.besti)
        self.f.write('        if (n >= 5) { if (iterate_%s_28_24(d+2, e+2, h+2, j+2, 0, (n == 5))) {return 6;} }\n' % self.besti)
        self.f.write('        if (n >= 3) { if (iterate_%s_24_20(d+4, e+4, h+4, j+4, 0, (n == 3))) {return 4;} }\n' % self.besti)
        self.f.write('        if (n >= 1) { if (iterate_%s_20_16(d+6, e+6, h+6, j+6, 0, (n == 1))) {return 2;} }\n' % self.besti)
        self.f.write('        return 0;\n')
        self.f.write('    }\n\n')

        self.f.write('    int %s(int n, uint32_t * __restrict__ d, uint32_t * __restrict__ h) {\n' % name)
        self.f.write('        uint32_t e[32];\n')
        self.f.write('        if (n >= 7) { if (iterate_%s_32_28(d, e, h, 0, 0, (n == 7))) {return 8;} }\n' % self.besti)
        self.f.write('        if (n >= 5) { if (iterate_%s_28_24(d+2, e+2, h+2, 0, 0, (n == 5))) {return 6;} }\n' % self.besti)
        self.f.write('        if (n >= 3) { if (iterate_%s_24_20(d+4, e+4, h+4, 0, 0, (n == 3))) {return 4;} }\n' % self.besti)
        self.f.write('        if (n >= 1) { if (iterate_%s_20_16(d+6, e+6, h+6, 0, 0, (n == 1))) {return 2;} }\n' % self.besti)
        self.f.write('        return 0;\n')
        self.f.write('    }\n\n')

        self.f.write('    int %s(int n, uint32_t * __restrict__ d) {\n' % name)
        self.f.write('        uint32_t e[32];\n')
        self.f.write('        if (n >= 7) { if (iterate_%s_32_28(d, e, 0, 0, 0, (n == 7))) {return 8;} }\n' % self.besti)
        self.f.write('        if (n >= 5) { if (iterate_%s_28_24(d+2, e+2, 0, 0, 0, (n == 5))) {return 6;} }\n' % self.besti)
        self.f.write('        if (n >= 3) { if (iterate_%s_24_20(d+4, e+4, 0, 0, 0, (n == 3))) {return 4;} }\n' % self.besti)
        self.f.write('        if (n >= 1) { if (iterate_%s_20_16(d+6, e+6, 0, 0, 0, (n == 1))) {return 2;} }\n' % self.besti)
        self.f.write('        return 0;\n')
        self.f.write('    }\n\n')

    def __init__(self, f, iset):
        self.f = f
        self.iset = iset
        for k in ['sse2', 'sse3', 'ssse3', 'sse4', 'avx', 'avx2']:
            if k in iset:
                self.besti = k

def gwli_bsi(f, bsi, msi):

    f.write('            apg::z64_to_r32_%s(inleaves, d);\n' % bsi)
    f.write('            apg::z64_to_r32_%s(hleaves, h);\n' % bsi)
    f.write('            iterate_var_%s(d, h);\n' % bsi)
    f.write('            apg::r32_centre_to_z64_%s(d, outleaf);\n' % msi)
    f.write('            apg::r32_centre_to_z64_%s(h, outleaf2);\n' % msi)

def wli_bsi(f, hist, bsi, msi):

    f.write('            apg::z64_to_r32_%s(inleaves, d);\n' % bsi)
    if (hist >= 2):
        f.write('            apg::z64_to_r32_%s(jleaves, j);\n' % bsi)
    if (hist >= 1):
        f.write('            apg::z64_to_r32_%s(hleaves, h);\n' % bsi)

    if (hist >= 2):
        f.write('            nochange = (iterate_var_%s(n, d, h, j) == n);\n' % bsi)
    elif (hist >= 1):
        f.write('            nochange = (iterate_var_%s(n, d, h) == n);\n' % bsi)
    else:
        f.write('            nochange = (iterate_var_%s(n, d) == n);\n' % bsi)

    f.write('            apg::r32_centre_to_z64_%s(d, outleaf);\n' % msi)
    if (hist >= 2):
        f.write('            apg::r32_centre_to_z64_%s(j, outleaf + 8);\n' % msi)
    if (hist >= 1):
        f.write('            apg::r32_centre_to_z64_%s(h, outleaf + 4);\n' % msi)

def write_all_iterators(f, hist, rules):

    params = 'int n, uint64_t * inleaves'
    params2 = 'uint32_t* d'
    xparams = 'n, inleaves'
    xparams2 = 'd, e'
    if (hist):
        params += ', uint64_t * hleaves'
        xparams += ', hleaves'
        params2 += ', uint32_t* h'
        xparams2 += ', h'
    else:
        xparams2 += ', 0'
    if (hist >= 2):
        params += ', uint64_t * jleaves'
        xparams += ', jleaves'
        params2 += ', uint32_t* j'
        xparams2 += ', j'
    else:
        xparams2 += ', 0'
    params += ', uint64_t * outleaf'
    params2 += ', uint32_t * diffs'
    xparams += ', outleaf'
    xparams2 += ', diffs, false'
    f.write('    int iterate_var_leaf(int rule, %s) {\n' % params)
    f.write('        switch(rule) {\n')

    not_used = True
    for (i, runsafe) in enumerate(rules):
        r = runsafe.replace('-', '_')
        m = re.match('b0?1?2?3?4?5?6?7?8?s0?1?2?3?4?5?6?7?8?$', runsafe)
        if m is not None:
            not_used = False
            f.write('            case %d :\n' % i)
            f.write('                return %s::iterate_var_leaf(%s);\n' % (r, xparams))
        elif ((r[0] == 'g') and (hist == 1)):
            not_used = False
            f.write('            case %d :\n' % i)
            f.write('                return %s::iterate_var_leaf(inleaves, hleaves, outleaf);\n' % r)
        elif ((r[0] != 'g') and (hist == 0)):
            not_used = False
            f.write('            case %d :\n' % i)
            f.write('                return %s::iterate_var_leaf(inleaves, outleaf);\n' % r)
    f.write('        }\n')
    if not_used:
        f.write('        (void) %s;\n' % (xparams.replace(',', '; (void)')))
    f.write('        return -1;\n')
    f.write('    }\n\n')

    f.write('    int iterate_var_32_28(int rule, %s) {\n' % params2)
    f.write('        uint32_t e[32];\n')
    f.write('        int bis = apg::best_instruction_set();\n')
    f.write('        switch(rule) {\n')

    not_used = True
    for (i, runsafe) in enumerate(rules):
        r = runsafe.replace('-', '_')
        m = re.match('b0?1?2?3?4?5?6?7?8?s0?1?2?3?4?5?6?7?8?$', runsafe)
        if m is not None:
            not_used = False
            f.write('            case %d :\n' % i)
            f.write('                if (bis >= 10) {\n')
            f.write('                    return %s::iterate_avx2_32_28(%s);\n' % (r, xparams2))
            f.write('                } else if (bis >= 9) {\n')
            f.write('                    return %s::iterate_avx_32_28(%s);\n' % (r, xparams2))
            f.write('                } else {\n')
            f.write('                    return %s::iterate_sse2_32_28(%s);\n' % (r, xparams2))
            f.write('                }\n')
    f.write('        }\n')
    if not_used:
        f.write('        (void) %s;\n' % (xparams2.replace(', 0', '').replace(', false', '').replace(',', '; (void)')))
        f.write('        (void) bis;\n')
    f.write('        return -1;\n')
    f.write('    }\n\n')

def write_leaf_iterator(f, hist):

    name = 'iterate_var_leaf'
    params = 'int n, uint64_t * inleaves'
    if (hist):
        params += ', uint64_t * hleaves'
    if (hist >= 2):
        params += ', uint64_t * jleaves'
    params += ', uint64_t * outleaf'
    f.write('    bool %s(%s) {\n' % (name, params))
    f.write('        bool nochange = false;\n')
    f.write('        int bis = apg::best_instruction_set();\n')
    f.write('        uint32_t d[32];\n')

    if (hist >= 1):
        f.write('        uint32_t h[32];\n')
    if (hist >= 2):
        f.write('        uint32_t j[32];\n')

    f.write('        if (bis >= 10) {\n')
    wli_bsi(f, hist, 'avx2', 'avx2')
    f.write('        } else if (bis >= 9) {\n')
    wli_bsi(f, hist, 'avx', 'avx')
    f.write('        } else if (bis >= 7) {\n')
    wli_bsi(f, hist, 'sse2', 'sse4')
    f.write('        } else {\n')
    wli_bsi(f, hist, 'sse2', 'ssse3')
    f.write('        }\n')
    f.write('        return nochange;\n')
    f.write('    }\n\n')

def gwrite_leaf_iterator(f, nstates):

    name = 'iterate_var_leaf'
    params = 'uint64_t * inleaves, uint64_t * hleaves, uint64_t * outleaf'
    f.write('    bool %s(%s) {\n' % (name, params))
    f.write('        uint64_t outleaf2[4];')
    f.write('        int bis = apg::best_instruction_set();\n')
    f.write('        uint32_t d[32];\n')
    f.write('        uint32_t h[32];\n')
    f.write('        if (bis >= 10) {\n')
    gwli_bsi(f, 'avx2', 'avx2')
    f.write('        } else if (bis >= 9) {\n')
    gwli_bsi(f, 'avx', 'avx')
    f.write('        } else if (bis >= 7) {\n')
    gwli_bsi(f, 'sse2', 'sse4')
    f.write('        } else {\n')
    gwli_bsi(f, 'sse2', 'ssse3')
    f.write('        }\n')

    br = '' if (nstates == 3) else (bin(nstates - 3)[2:])[::-1]

    # We run 256 parallel binary counters in outleaf:
    f.write('        for (int i = 0; i < 4; i++) {\n')
    f.write('            uint64_t carry = outleaf[4+i];\n')
    for i, c in enumerate(br):
        f.write('            outleaf[%d+i] ^= carry;\n' % (4 * i + 8))
        f.write('            carry &= outleaf[%d+i];\n' % (4 * i + 8))
        if (c == '1'):
            f.write('            outleaf[%d+i] |= outleaf2[i];\n' % (4 * i + 8))
        # else:
        #     f.write('            outleaf[%d+i] &= outleaf2[i];\n' % (4 * i + 8))
    f.write('            outleaf[4+i] ^= carry;\n')
    for i, c in enumerate(br):
        f.write('            outleaf[%d+i] ^= carry;\n' % (4 * i + 8))
    f.write('            outleaf[4+i] |= outleaf2[i];\n')
    f.write('        }\n')

    f.write('        return false;\n')
    f.write('    }\n\n')

def makeltl(rulestring, gparams):

    if not os.path.exists('lifelogic'):
        os.makedirs('lifelogic')

    with open('lifelogic/iterators_%s.h' % rulestring, 'w') as f:
        f.write('#pragma once\n')
        f.write('#include <stdint.h>\n')
        f.write('#include "../ltl.h"\n')
        f.write('namespace %s {\n\n' % rulestring.replace('-', '_'))

        f.write('    bool iterate_var_leaf(uint64_t *inleaves, uint64_t *outleaf) {\n\n')
        f.write('        apg::ltl_kernel(inleaves, outleaf')
        for g in gparams:
            f.write(', %d' % g)
        f.write(');\n')
        f.write('        return false;\n')
        f.write('    }\n')
        f.write('}\n')

def makeiso(rulestring):

    if not os.path.exists('lifelogic'):
        os.makedirs('lifelogic')

    logstring = rulestring[rulestring.index('b'):]
    '''
    for iset in [['sse2'], ['sse2', 'avx'], ['sse2', 'avx', 'avx2']]:
        with open(('lifelogic/llma_%s.asm' % iset[-1]), 'w') as f:
            ix = iwriter(f, iset)
            ix.isogen()
    '''

    with open('lifelogic/iterators_%s.h' % rulestring, 'w') as f:
        f.write('#pragma once\n')
        f.write('#include <stdint.h>\n')
        f.write('#include <cstdlib>\n')
        f.write('#include <iostream>\n')
        f.write('#include "../eors.h"\n')
        f.write('#include "../lifeperm.h"\n')
        f.write('namespace %s {\n\n' % rulestring.replace('-', '_'))
        f.write('#include "ma_%s.h"\n\n' % rulestring)
        f.write('#include "../isoluts.h"\n')
        f.write('}\n')

def makeasm(rulestring):

    if not os.path.exists('lifelogic'):
        os.makedirs('lifelogic')

    logstring = rulestring[rulestring.index('b'):]
    for iset in [['sse2'], ['sse2', 'avx'], ['sse2', 'avx', 'avx2']]:
        with open('lifelogic/ll_%s_%s.asm' % (iset[-1], logstring), 'w') as f:
            ix = iwriter(f, iset)
            ix.genlogic(logstring)

    with open('lifelogic/iterators_%s.h' % rulestring, 'w') as f:
        f.write('#pragma once\n')
        f.write('#include <stdint.h>\n')
        f.write('#include "../lifeconsts.h"\n')
        f.write('#include "../lifeperm.h"\n')
        f.write('#include "../eors.h"\n')
        f.write('namespace %s {\n\n' % rulestring.replace('-', '_'))

        '''
        for (x, y) in [(28, '0x3ffffffcu'), (24, '0x0ffffff0u'), (20, '0x03ffffc0u'), (16, '0x00ffff00u')]:
            f.write('    const static uint32_t __sixteen%d[] __attribute__((aligned(64))) = {' % x)
            for i in xrange(8):
                f.write('%s,\n        ' % y)
            f.write('1, 2, 3, 4, 5, 6, 7, 0};\n\n')
        '''

        for iset in [['sse2'], ['sse2', 'avx'], ['sse2', 'avx', 'avx2']]:
            iw = iwriter(f, iset)
            if (rulestring[0] == 'g'):
                iw.gwrite_function(rulestring, 20, 16)
                iw.gwrite_iterator()
            else:
                iw.write_function(rulestring, 32, 28)
                iw.write_function(rulestring, 28, 24)
                iw.write_function(rulestring, 24, 20)
                iw.write_function(rulestring, 20, 16)
                iw.write_iterator()

        if (rulestring[0] == 'g'):
            gwrite_leaf_iterator(f, int(rulestring[1:rulestring.index('b')]))
        else:
            for hist in xrange(3):
                write_leaf_iterator(f, hist)
        f.write('}\n')

def main():

    os.chdir(get_script_path())
    try:
        os.makedirs('lifelogic')
    except OSError as e:
        print(repr(e))

    rules = list(sys.argv)[1:]

    if (len(rules) == 0):
        print("Usage:")
        print("python rule2asm.py b3s23 b38s23 g4b2s345")
        exit(1)
    if (len(rules) > 8):
        print("apglib supports a maximum of 8 different rules")
        exit(1)

    for rulestring in rules:

        m = re.match('(g[1-9][0-9]*)?b0?1?2?3?4?5?6?7?8?s0?1?2?3?4?5?6?7?8?$', rulestring)
        if m is not None:
            if (rulestring[0] == 'g') and (int(rulestring[1:rulestring.index('b')]) <= 2):
                print("Generations rules must have at least 3 states.")
                exit(1)
            if (rulestring[0] == 'g') and ('b0' in rulestring):
                print("Generations rules must not have birth on zero neighbours.")
                exit(1)
            if ('b0' in rulestring) and (rulestring[-1] == '8'):
                print("A rule cannot contain both b0 and s8.")
                exit(1)
            print("Valid rulestring: \033[1;32m"+m.group(0)+"\033[0m")
            makeasm(rulestring)
            continue

        m = re.match('r[234567]b[1-9][0-9]*t[1-9][0-9]*s[1-9][0-9]*t[1-9][0-9]*$', rulestring)
        if m is not None:
            gparams = rulestring[1:].replace('b', 't').replace('s', 't').split('t')
            gparams = tuple(map(int, gparams))
            if (gparams[1] > gparams[2]):
                print("Minimum birth count cannot exceed maximum birth count")
                exit(1)
            if (gparams[3] > gparams[4]):
                print("Minimum survival count cannot exceed maximum survival count")
                exit(1)
            if (gparams[2] >= ((2 * gparams[0] + 1) ** 2)):
                print("Maximum birth count cannot exceed or equal neighbourhood area")
                exit(1)
            if (gparams[4] > ((2 * gparams[0] + 1) ** 2)):
                print("Maximum survival count cannot exceed neighbourhood area")
                exit(1)
            print("Valid rulestring: \033[1;32m"+m.group(0)+"\033[0m")
            makeltl(rulestring, gparams)
            continue

        m = re.match('b[0-9a-z-]*s[0-9a-z-]*$', rulestring)
        if m is not None:
            status = os.system('python ntcanon.py "%s"' % rulestring)
            if (status != 0):
                exit(status)
            if (rulestring[1] == '0'):
                print("Non-totalistic B0 rules are not supported by lifelib")
                exit(1)
            makeiso(rulestring)
            continue

        print("Invalid rulestring: \033[1;31m"+rulestring+"\033[0m is not of the form:")
        print("    bXsY\nwhere X, Y are subsets of {0, 1, 2, ..., 8}, or:")
        print("    gNbXsY\nwhere N >= 3 is the number of states, or:")
        print("    rNbWtXsYtZ\nwhere 2 <= N <= 7 is the range and W,X,Y,Z are positive integers")
        exit(1)

    with open('lifelogic/iterators_all.h', 'w') as f:

        f.write('#pragma once\n')
        f.write('#include <string>\n')
        for rulestring in rules:
            f.write('#include "iterators_%s.h"\n' % rulestring)
        f.write('namespace apg {\n\n')
        f.write('    int rule2int(std::string rule) {\n')
        for (i, r) in enumerate(rules):
            f.write('        if (rule == "%s") { return %d; }\n' % (r, i))
        f.write('        return -1;\n')
        f.write('    }\n\n')
        for hist in xrange(3):
            write_all_iterators(f, hist, rules)

        f.write('    int uli_get_family(int rule) {\n')
        f.write('        switch (rule) {\n')
        for (i, r) in enumerate(rules):
            m = re.match('b0?1?2?3?4?5?6?7?8?s0?1?2?3?4?5?6?7?8?$', rulestring)
            f.write('            case %d :\n' % i)
            if m is not None:
                f.write('                return 0;\n')
            elif (r[0] == 'g'):
                f.write('                return 2;\n')
            else:
                f.write('                return 4;\n')
        f.write('        }\n')
        f.write('        return 0;\n')
        f.write('    }\n\n')

        f.write('    uint64_t uli_valid_mantissa(int rule) {\n')
        f.write('        switch (rule) {\n')
        for (i, r) in enumerate(rules):
            m = re.match('b0?1?2?3?4?5?6?7?8?s0?1?2?3?4?5?6?7?8?$', rulestring)
            f.write('            case %d :\n' % i)
            if m is None:
                f.write('                return 3;\n')
            elif (r[:2] == 'b0'):
                f.write('                return 5;\n')
            else:
                f.write('                return 255;\n')
        f.write('        }\n')
        f.write('        return 3;\n')
        f.write('    }\n\n')
        f.write('}\n')

main()
