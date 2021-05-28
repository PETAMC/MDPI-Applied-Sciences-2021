#include <iostream>
#include <histogram.hpp>
#include <cmath>

Histogram::Histogram(std::string &path)
    : Distribution(path.c_str())
    , label(path)
    , color("#808080")
{
}

Histogram::Histogram(std::string &label, std::string &color, std::string &path)
    : Distribution(path.c_str())
    , label(label)
    , color(color)
{
}


Histogram::~Histogram()
{
}



const BinDefinition& Histogram::DefineBins(unsigned int numofbins)
{
    this->bindef.size = (this->max - this->min) / numofbins;
    this->bindef.from = this->min;
    this->bindef.to   = this->bindef.from + (numofbins + 1) * this->bindef.size;

    // Handle some exceptions:
    if(this->max == this->min)
    {
        std::cerr << "\e[1;33mWARNING: all samples for histogram \e[1;37m" << this->label << "\e[1;33m are equal.\n";
        std::cerr << "\e[1;33m → Bin size forced to max to avoid miscalculation of common bin size.\n";

        this->bindef.size = std::numeric_limits<long>::max();
    }
    if(this->bindef.size <= 0)
    {
        std::cerr << "\e[1;33mWARNING: bin size less than 1 for histogram \e[1;37m" << this->label << "\e[1;33m.\n";
        std::cerr << "\e[1;33m → Bin size forced to be 1.\n";
        this->bindef.size = 1;
    }

    return this->bindef;
}



const BinDefinition& Histogram::GetBinDefinition() const
{
    return this->bindef;
}



const std::string& Histogram::GetColor() const
{
    return this->color;
}



const std::string& Histogram::GetLabel() const
{
    return this->label;
}



const std::map<long, float>& Histogram::GetRawData() const
{
    return this->histogram;
}



void Histogram::RedefineBins(const BinDefinition &newdefinition)
{
    this->bindef = newdefinition;
}



void Histogram::CreateHistogram()
{
    // Clear old histogram
    this->histogram.clear();

    // Create Bins
    long numsamples = 0;
    for(const auto &entry : this->counts)
    {
        long value;
        long count;
        long bin;

        value       = entry.first;
        count       = entry.second;
        bin         = (value - this->bindef.from) / this->bindef.size;
        numsamples += count;

        this->histogram[bin] += static_cast<float>(count);
    }

    // Normalize bins
    for(auto &entry : this->histogram)
    {
        entry.second /= numsamples;
    }

    return;
}



double Histogram::BhattacharyyaDistance(const Histogram &other)
{
    // Get raw histograms
    const auto &hista = this->GetRawData();
    const auto &histb = other.GetRawData();

    // Figure out which histogram has less bins
    int sizea = hista.size();
    int sizeb = histb.size();

    int maxiter   = (sizea < sizeb) ? sizea : sizeb;

    // Convert histograms to probability vectors
    std::vector<float> pva;
    std::vector<float> pvb;

    for(const auto &entry : hista)
        pva.push_back(entry.second);
    for(const auto &entry : histb) 
        pvb.push_back(entry.second);

    // Calculate Bhattacharyya coefficient
    double bhattacharyya = 0.0;

    for(int i=0; i<maxiter; i++)
    {
        // Get Probability a and b
        float pa = pva[i];
        float pb = pvb[i];

        // Calculate coefficient and integrate
        double c = std::sqrt(pa * pb);
        bhattacharyya += c;
    }

    // Calculate Bhattacharyya distance
    double distance;

    if(bhattacharyya == 0.0)
        distance = std::numeric_limits<double>::infinity();
    else
        distance = - std::log(bhattacharyya);

    return distance;
}





