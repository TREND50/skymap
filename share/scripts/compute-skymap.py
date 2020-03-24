#!/usr/bin/env python
#===============================================================================
# Grid Engine steering options
#===============================================================================
## Submit the job under MIM group:
#$ -P P_trend

## Set the job name:
#$ -N skymap
## Set the task ID:
#$ -t 5:90:5

## Merge the stdout and stderr in a single file:
#$ -j y
## Files .e et .o are copied to the current working directory:
#$ -cwd
## Notify the stop and kill signals before issuing them:
#$ -notify

## The requested CPU time:
#$ -l ct=02:00:00
## The requested memory:
#$ -l vmem=3.0G
## The requested disk space:
#$ -l fsize=1.0G
## Request /sps/:
#$ -l sps=1
#===============================================================================
import cPickle as pk
import os
import subprocess as sp
import sys
import time
import numpy as np

# Parse arguments.
tid = os.getenv("SGE_TASK_ID")
if tid:
        polarisation = float(tid)
else:
        try:
                polarisation = float(sys.argv[1])
        except:
                polarisation= 0.

# Settings.
data_dir = "/afs/in2p3.fr/home/n/niess/trend/skymap/share/data"
frequencies = np.linspace(50., 100., 51)

def compute_flux(frequency):
        t0 = time.time()
        p = sp.Popen("./bin/skymap -f{:.5E} -p{:.5E}".format(frequency,
                polarisation), stdout=sp.PIPE, stderr=sp.PIPE, shell=True)
        stdout, stderr = p.communicate()
        if stderr:
                raise RuntimeError(stderr)
        data = stdout.split("\n")[:-1]
        data = np.array([map(float, v.split()) for v in data])
        dt = time.time()-t0
        return data, dt

def integrate_flux(lst):

        y = np.zeros(frequencies.shape)
        for i, fi in enumerate(frequencies):
                y[i] = compute_flux(fi, lst)

        return np.trapz(y, frequencies)*1E+06, dt

if __name__ == "__main__":
        flux = []
        for fi in frequencies:
                print "o) Processing flux at {:.0f} MHz".format(fi)
                data, dt = compute_flux(fi)
                flux.append(data[:,1])
                lst = data[:,0]
                print "  --> Done in {:.0f} s".format(dt)

        flux = np.array(flux)
        with open(os.path.join(data_dir, "skymap.{:02.0f}.50.p".format(polarisation)), "wb+") as f:
                pk.dump({"frequencies": frequencies, "lst": lst, "flux": flux,
                        "polarisation": polarisation}, f, -1)
