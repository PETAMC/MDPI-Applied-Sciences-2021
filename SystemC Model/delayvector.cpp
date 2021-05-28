#include <delayvector.hpp>
#include <cstdlib>
#include <iostream>
//#include <filesystem>
#include <sys/types.h>
#include <sys/stat.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_statistics_double.h>
#include <setup/pythonwrapper.hpp>


DelayVector::DelayVector(const char *path, DISTRIBUTION distribution, unsigned int offset)
    : delayindex(0)
    , rng(nullptr)
    , distribution(distribution)
    , datapath(path)
    , offset(offset)
{
    //this->InitializeData();
}

DelayVector::DelayVector(std::string path, DISTRIBUTION distribution, unsigned int offset)
    : delayindex(0)
    , rng(nullptr)
    , distribution(distribution)
    , datapath(path)
    , offset(offset)
{
    //this->InitializeData();
}

void DelayVector::InitializeData()
{
    // Check if file exists
    ifstream f(this->datapath);
    if(not f.good())
    {
        std::cerr << "\e[1;33mMissing data: \e[1;34m"
                  << this->datapath
                  << "\e[0m\n";
        return;
    }
    else
        f.close();

    // Read file
    if(this->distribution == DISTRIBUTION::GAUSSIAN_KDE)
    {
        this->ReadKDEFittedDelayVector();
    }
    else
    {
        this->ReadDelayVector();
    }
    this->rng = gsl_rng_alloc(gsl_rng_default);
    // By default, the random number generator gets seeded with 0
    // This shall not be changed to get reproducible results
}



sc_core::sc_time DelayVector::GetDelay()
{
    // When this function gets called the first time,
    // the delay vector gets read from the file.
    // This is done here to avoid loading lots of data
    // that will never be used during the simulation.
    if(this->delayvector.empty())
    {
        this->InitializeData();
        this->GenerateOffsetDelays();
    }

    double delay;

    // The KDE just fits the data to a set of steady functions.
    // After fitting, the result is still a vector of delays,
    // and therefor can be handled as injected data.
    switch(this->distribution)
    {
        case DISTRIBUTION::INJECTED:
        case DISTRIBUTION::GAUSSIAN_KDE:
            delay = this->GetInjectedDelay();
            break;

        case DISTRIBUTION::GAUSSIAN:
            delay = this->GetGaussianDelay();
            break;

        case DISTRIBUTION::UNIFORM:
            delay = this->GetUniformDelay();
            break;

        case DISTRIBUTION::WCET:
            delay = this->GetWCETDelay();
            break;

        case DISTRIBUTION::AVERAGE:
            delay = this->GetAverageDelay();
            break;
    }

    return sc_core::sc_time(delay, sc_core::SC_NS);
}


void DelayVector::GenerateOffsetDelays()
{
    for(unsigned int i=0; i<offset; i++)
        this->GetDelay();
    return;
}


double DelayVector::GetInjectedDelay()
{
    double delay = this->delayvector[this->delayindex];
    this->delayindex = (this->delayindex + 1) % this->delayvector.size();

    return delay;
}



double DelayVector::GetGaussianDelay()
{
    auto delay = gsl_ran_gaussian(this->rng, this->sigma);
    delay += this->mu;

    return delay;
}



double DelayVector::GetUniformDelay()
{
    auto delay = gsl_ran_flat(this->rng, this->BCET, this->WCET);

    return delay;
}



double DelayVector::GetWCETDelay()
{
    return this->WCET;
}



double DelayVector::GetAverageDelay()
{
    return this->mu;
}



void DelayVector::ReadDelayVector()
{
    std::ifstream ifs;
    ifs.open(this->datapath);

    // Initialize min/max with opposite extremes
    // to approach to the actual values from the file
    this->WCET = 0.0;
    this->BCET = std::numeric_limits<decltype(this->BCET)>::max();

    for(std::string line; std::getline(ifs, line); )
    {
        double time;
        try
        {
            time = std::stod(line);
        }
        catch(std::exception &e)
        {
            std::cerr << "Parsing line \""
                      << line
                      << "\" failed."
                      << "This line will be ignored!\n";
        }

        this->delayvector.emplace_back(time);
        if(time > this->WCET)
            this->WCET = time;
        if(time < this->BCET)
            this->BCET = time;

    }

    this->PostProcessDelayVector();

    ifs.close();
    return;
}


void DelayVector::ReadKDEFittedDelayVector()
{
    // Check, if there is already cached data
    //size_t datapathhash = std::hash<std::string>{}(this->datapath);
    /*
     * FIXME: This code requires C++17. Currently we are limited to C++14 because of SystemC
    std::filesystem::path temppath = std::filesystem::temp_directory_path();
    temppath += std::to_string(datapathhash) + ".txt";
    * WORKAROUND The following code is an alternative
    */
    //std::string cachepath = std::string(secure_getenv("HOME")) + "/.smcdelaycache";
    //std::string cachefile = std::to_string(datapathhash)       + ".txt";



    /* TODO: This is how to create a new directory. Necessary for storing the cached KDE
    status = mkdir(cachepath.c_str(), S_IRWXU | S_IRG | S_IXG | S_IROTH | S_IXOTH); // Try to create cache directory
    if(status != 0)
    {
        switch(errno)
        {
            case EEXIST: // The directory already exists. This is good.
                break
            case EACCES:
            case ELOOP:
            case EMLINK:
            case ENAMETOOLONG:
            case ENOENT:
            case ENOSPC:
            case ENOTDIR:
            case EROFS:
            default:
                std::cerr << "\e[1;33mWARNING:\e[0m Failed to create \""
                    << temppath << "\" - "
                    << strerror(errno)
                    << "\e[1;30m(Trying to proceed anyway)\e[0m\n";
        }
    }
    */


    // Apply KDE
    PythonWrapper &pythonwrapper = PythonWrapper::GetInstance();
    pythonwrapper.GaussianKDE(&this->delayvector, this->datapath);

    // Initialize min/max with opposite extremes
    // to approach to the actual values from the file
    this->WCET = 0.0;
    this->BCET = std::numeric_limits<decltype(this->BCET)>::max();

    for(size_t i = 0; i < this->delayvector.size(); i++)
    {
        double time;
        time = this->delayvector[i];
        if(time > this->WCET)
            this->WCET = time;
        if(time < this->BCET)
            this->BCET = time;
    }

    this->PostProcessDelayVector();
    return;
}

void DelayVector::PostProcessDelayVector()
{
    // Shuffle delay vector to make sure it is not just a repetition of the measured data
    std::srand(0);  // Make sure the random number is not depending on call-order
    std::random_shuffle(this->delayvector.begin(), this->delayvector.end());

    // Get further properties from the measured data
    double *data = this->delayvector.data();
    size_t  n    = this->delayvector.size();
    this->sigma  = gsl_stats_sd(  data, 1, n);
    this->mu     = gsl_stats_mean(data, 1, n);
    return;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

