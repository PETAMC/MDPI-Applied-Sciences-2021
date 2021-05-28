#ifndef DISTRIBUTION_HPP
#define DISTRIBUTION_HPP

#include <vector>
#include <map>
#include <string>

class Distribution
{
    public:
        Distribution(const char *path);
        ~Distribution();

        void ProcessSamples();
        long GetMaximum() const;
        long GetAverage() const;

        long ParseLine(const std::string &line) const;

    protected:
        // Processed Data
        long                 min;
        long                 max;
        long long            sum;
        long                 avg;
        std::map<long, long> counts;    // <value, count>

    private:
        int LoadSamples(const char *path);
        // Loads integer (long) sampled from file into this->data.
        // In case of an error -1 is returned and an error message printed to stderr,
        // otherwise 0 is returned.

        // Raw Data
        std::vector<long> data;
};

#endif

