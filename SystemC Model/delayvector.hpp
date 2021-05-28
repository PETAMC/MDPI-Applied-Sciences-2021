#ifndef DELAYVECTOR_HPP
#define DELAYVECTOR_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <systemc.h>
#include <cstdlib>
#include <gsl/gsl_rng.h>

enum DISTRIBUTION
{
    INJECTED,
    GAUSSIAN,
    UNIFORM,
    WCET,
    AVERAGE,
    GAUSSIAN_KDE
};

class DelayVector;
typedef std::unordered_map<std::string, DelayVector*> DelayVectorMap;

// Important:
// Internal, this class works with doubles for the delays.
// A delay's unit is clock cycle.
// The external interfaces should be of type sc_time

class DelayVector
{
    public:
        DelayVector(const char *path, DISTRIBUTION distribution = DISTRIBUTION::INJECTED, unsigned int offset = 0);
        DelayVector(std::string path, DISTRIBUTION distribution = DISTRIBUTION::INJECTED, unsigned int offset = 0);

        sc_core::sc_time GetDelay();

        void InitializeData(); // When calling GetDelay the first time,
                               // this method will be called automatically.
                               // Only call this method when you know
                               // before simulation, that you need will
                               // use the data.

    private:
        void GenerateOffsetDelays();
        
        double GetInjectedDelay();
        double GetGaussianDelay();
        double GetUniformDelay();
        double GetWCETDelay();
        double GetAverageDelay();

        void ReadDelayVector();
        void ReadKDEFittedDelayVector();
        void PostProcessDelayVector();



        // This vector holds all measured delay
        // In case of GAUSSIAN_KDE these values were pre-processed
        std::vector<double> delayvector;

        // These variables are properties of the measured delay
        double BCET;  // Lowest delay
        double WCET;  // Highest delay
        double sigma;
        double mu;    // µ / average

        // Some variables for accessing the delays
        unsigned int delayindex; // Next delay in delay vector
        gsl_rng *rng;   // Random number generator for Normal/Uniform distribution

        std::string  datapath;
        DISTRIBUTION distribution;

        unsigned int offset;
};

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

