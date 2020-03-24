#!/usr/bin/env python
import time
import numpy as np
import pylab as pl
from matplotlib.colors import LogNorm
import healpy as hp

with open("share/data/0050.dat", "r") as f:
        hpm = map(float, f.read().split("\n")[:-1])

NSIDE = 512
scale = 1
de = np.linspace(-90, 90., 180*scale+1)
ra = np.linspace(0., 24., 360*scale+1)

print "o) Computing 2D map ..."
m = np.zeros((len(de), len(ra)))
t0 = time.time()
for i, dei in enumerate(de):
        for j, raj in enumerate(ra):
                tg, pg = hp.Rotator(coord='cg')((90.-dei)*np.pi/180.,
                        raj*np.pi/12.)
                n = hp.get_all_neighbours(NSIDE, tg, pg)
                n = n[n >= 0]
                tt, pp = hp.pix2ang(NSIDE, n)
                w = (tt-tg)**2+(pp-pg)**2
                w /= sum(w)
                mm = np.array([hpm[nn] for nn in n])
                m[i, j] = sum(mm*w)
        dt = time.time()-t0
        print " . {:3d} / {:} done in {:.0f} s".format(i+1, len(de), dt)
print "  --> All done!"

kB = 1.38064852E-23
lbd = 3E+08/11.5E+09
A = 0.25 * 1.512 * 1.610 *np.pi
Omega = lbd**2 / A
m *= 2. * kB / lbd**2 * Omega / 1E-26

pl.figure()
pl.pcolor(ra, de, m, norm=LogNorm())
pl.colorbar()
pl.axis((0., 24., -90., 90.))
pl.xticks(np.linspace(0., 24., 7))
pl.yticks(np.linspace(-90., 90., 7))
pl.grid(True)

pl.show()
