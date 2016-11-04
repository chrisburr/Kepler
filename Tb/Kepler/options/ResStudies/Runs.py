import os


class Run():
    def __init__(self, r, a, b, d, bl, ad, aod):
        self.RUN       = r
        self.ANGLE     = a
        self.BIAS      = b
        self.DUT       = d
        self.BLOCK     = bl
        absad = os.path.abspath( ad )
        if not os.path.exists(absad):
            os.system('mkdir -p %s' % absad)
        absaod = os.path.abspath( aod )
        if not os.path.exists(absaod):
            os.system('mkdir -p %s' % absaod)
        self.ALIGNFILE = absad+'/Alignment'+self.RUN+'.dat'
        self.ALIGNOUTFILE = absaod+'/Alignment'+self.RUN+'.dat' 

    def createAlignFile(self):
        devs = ['', 'W0002_J06','W0002_B05','W0002_C05','W0002_D05',self.DUT,'W0002_G05','W0002_F05','W0002_H07','W0002_E05']
        s = ''
        zfile = open('ZPos.txt','r')
        for ll in zfile.readlines():
            l = ll.split()
            if '#' in l[0]:
                continue
            if l[0] == self.BLOCK:
                z  = l
        for i in range(9):
            if i < 4:
                s+= '%s 14.03 14.03 %s 2.9845 3.299 0.\n' %  (devs[i+1], z[i+1])
            elif i == 4:
                s+= '%s 0.0 14.03 %s 3.141 %f 0.\n' %  (devs[i+1], z[i+1], float(self.ANGLE)*3.141/180. )
            else:
                s+= '%s 0.0 14.03 %s 3.299 0.157 0.\n' %  (devs[i+1], z[i+1])

        af = open(self.ALIGNFILE, 'w')
        af.write(s)
        af.close

class Runs():
    def __init__(self, b, type):
        self.BLOCKS = b
        if type == 'survey':
            ad = 'init'
        elif type == 'mille':
            ad = 'survey'
        elif type == 'dut':
            ad = 'mille'
        else:
            ad = 'dut'
        self.TYPE        = type
        self.ALIGNDIR    = "Alignments/%s/" % (ad)
        print self.ALIGNDIR
        self.ALIGNOUTDIR = "Alignments/%s/" % (type) 
        self.RUNS        = self.defRuns()
        self.OUTPUTDIR   = ''

    def defRuns(self):
        rs = {}
        f  = open('runList.txt','r')
        for ll in f.readlines():
            l = ll.split()
            if '#' in l[0]:
                continue
            r = l[0]
            a = l[1]
            b = l[2]
            d = l[3]
            bl= l[4]
            if bl in self.BLOCKS:
                run = Run(r,a,b,d, bl, self.ALIGNDIR, self.ALIGNOUTDIR)
                if bl in rs:
                    rs[bl].append(run)
                else:
                    rs[bl] = [run]
        f.close()
        return rs

    def findRun(self, rn):
        for b in self.RUNS:
            for r in self.RUNS[b]:
                if r.RUN == rn:
                    return r
    
    def setOutputDir(self, od):
        self.OUTPUTDIR = od
