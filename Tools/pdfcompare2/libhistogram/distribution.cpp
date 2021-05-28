#include <distribution.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <limits>

Distribution::Distribution(const char *path)
{
    this->LoadSamples(path);
}

Distribution::~Distribution()
{
}


long Distribution::ParseLine(const std::string &line) const
{
    size_t begin = line.find_first_of(".");
    size_t end   = line.find_last_of("e");

    if(begin == std::string::npos and end == std::string::npos) // Normal number
    {
        return std::stol(line);
    }
    else if(begin != std::string::npos and end != std::string::npos) // Normal number
    {
        std::string number;   // ??.
        std::string fraction; // .??
        std::string exponent; // e+??
        std::string zeros;

        // Get number in front of "."
        number   = line.substr(0, begin);
        begin++;    // Skip "."

        // Get number behind "."
        fraction = line.substr(begin, end-begin);
        end++;   // Skip "e"
        if(line.at(end) == '+')
            end++; // Skip "+"

        // Get exponent
        exponent = line.substr(end);
        zeros    = std::string(std::stoi(exponent) - fraction.size(), '0');

        // Create integer
        std::string integer;
        integer = number + fraction + zeros;

        return std::stol(integer);
    }

    throw std::runtime_error("Invalid number format");
}


int Distribution::LoadSamples(const char *path)
{
    // Open File
    std::ifstream file(path);
    if(!file)
    {
        std::cerr << "\e[1;31mError: Unable to open " << path << "\e[0m\n";
        return -1;
    }

    // Read File
    std::string line;
    while(std::getline(file, line))
    {
        // Encode line to integer
        long sample;
        try
        {
            sample = this->ParseLine(line);
        }
        catch(const std::exception &e)
        {
            std::cerr << "\e[1;31mError: Parsing line \"" << line 
                      << "\" failed with exception \"" << e.what() 
                      << "\"! One integer sample per line is expected.\e[0m\n";
            return -1;
        }

        // Push sample into data set
        try
        {
            this->data.push_back(sample);
        }
        catch(const std::exception &e)
        {
            std::cerr << "\e[1;31mError: Storing sample in buffer"
                      << " failed with exception \"" << e.what() 
                      << "\"\e[0m\n";
            return -1;
        }
    }

    // Clean
    file.close();
    return 0;
}



void Distribution::ProcessSamples()
{
    if(this->data.size() == 0)
        return;

    // Reset processed data
    this->min = std::numeric_limits<long>::max();
    this->max = std::numeric_limits<long>::min();
    this->sum = 0;
    this->avg = 0;
    this->counts.clear();

    // Process data
    for(long sample : this->data)
    {
        this->counts[sample]++;
        this->sum += sample;

        if(sample < this->min)
            this->min = sample;
        if(sample > this->max)
            this->max = sample;
    }

    this->avg = this->sum / this->data.size();
}

long Distribution::GetMaximum() const
{
    return this->max;
}
long Distribution::GetAverage() const
{
    return this->avg;
}






