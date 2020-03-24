#!/usr/bin/env python
import os
import subprocess
import time

def run(frequency):
        print "o) Processing {:.0f} MHz".format(frequency)
        t0 = time.time()
        cmd = "./bin/gsm"
        p = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, shell=True)

        filename = "{:04.0f}.dat".format(frequency)
        args = "{:.0f}\n{:}".format(frequency, filename)
        stdout, stderr = p.communicate(args)
        if stderr:
                raise RuntimeError(stderr)
        os.rename(filename, os.path.join("share", "data", filename))
        dt = time.time()-t0
        print "  --> Done in {:.0f} s".format(dt)

if __name__ == "__main__":
        for i in xrange(20, 131):
                frequency = float(i)
                if os.path.exists("share/data/{:04.0f}.dat".format(frequency)):
                        continue
                run(frequency)
