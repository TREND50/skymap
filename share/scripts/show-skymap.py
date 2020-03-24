#!/usr/bin/env python
import cPickle as pk
import numpy as np
import pylab as pl

with open("share/data/skymap.ew.50.p", "rb") as f:
	globals().update(pk.load(f))

power = 0.5*np.trapz(flux, frequencies, axis=0)*1E+06

RL = 50.
kB = 1.38E-23
T = 297.

pl.figure()
pl.plot(lst, power, "k-")
pl.plot(lst, kB*T*50E+06*np.ones(lst.shape), "r-")
pl.grid(True)

pl.figure()
i0 = np.argmin(np.absolute(lst-18.))
pl.plot(frequencies, 0.5*flux[:, i0]*2.*RL, "k-")
pl.plot(frequencies, 4.*RL*kB*T*np.ones(frequencies.shape), "r-")
pl.grid(True)

pl.figure()
V2 = 0.5*np.trapz(flux*RL+4.*RL*kB*T, frequencies, axis=0)*1E+06
pl.plot(lst, V2, "k-")
pl.grid(True)

pl.show()
