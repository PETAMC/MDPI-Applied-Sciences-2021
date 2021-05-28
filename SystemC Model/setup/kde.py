#!/usr/bin/env python3

from sklearn.neighbors import KernelDensity
import numpy

def ReadSamplesFile(path):
    # Read Samples File
    with open(path) as f:
        lines = f.read().splitlines()

    # Decode Samples (Unicode numbers to NumPy Array)
    data   = [ int(line.strip()) for line in lines ]
    npdata = numpy.asarray(data)

    return npdata



def PerformKDE(samples, bandwidth=10000, kernel="gaussian"):
    kde = KernelDensity(kernel=kernel, bandwidth=bandwidth)
    kde.fit([samples])
    fit = kde.sample()[0]
    fit = [ int(point) for point in fit ]
    return fit



def GaussianKDE(samplespath):
    if type(samplespath) is not str:
        raise TypeError("Argument must be a path of type str! (Type was %s)", type(samplespath))

    numpy.random.seed(1)
    samples     = ReadSamplesFile(samplespath)
    delayvector = PerformKDE(samples, bandwidth=(0.01*max(samples))) # bandwidth = 1%

    return delayvector

if __name__ == "__main__":
    import os
    import sys
    
    inpath  = os.path.abspath(sys.argv[1])
    outpath = os.path.abspath(sys.argv[2])

    delayvector = GaussianKDE(inpath)

    with open(outpath, "w") as f:
        for delay in delayvector:
            f.write("%i\n" % (delay))


