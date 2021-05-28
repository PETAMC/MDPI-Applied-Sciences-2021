#ifndef HISTOGRAM_HPP
#define HISTOGRAM_HPP

#include <string>
#include <distribution.hpp>

//      ██
//   ██ ██   
//  ▁██▁██▁██▁
//   ▏  ▏▕  ▕
//   ▏ size ▕
// from     to
struct BinDefinition
{
    long from;
    long to;
    long size;
};

class Histogram : public Distribution
{
    public:
        Histogram(std::string &path);
        Histogram(std::string &label, std::string &color, std::string &path);
        ~Histogram();

        const BinDefinition& DefineBins(unsigned int numofbins);
        // Calculate bin definition based on the loaded distribution

        const BinDefinition& GetBinDefinition() const;
        const std::string&   GetColor() const;
        const std::string&   GetLabel() const;

        void RedefineBins(const BinDefinition &newdefinition);
        // Update bin definitions with customized settings

        void CreateHistogram();
        const std::map<long, float>& GetRawData() const;

        double BhattacharyyaDistance(const Histogram &other);

    private:
        std::string label;
        std::string color;
        BinDefinition bindef;

        std::map<long, float> histogram;
};

#endif

