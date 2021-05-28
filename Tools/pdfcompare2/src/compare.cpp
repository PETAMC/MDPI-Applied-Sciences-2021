#include <iostream>
#include <vector>
#include <cstdlib>
#include <histogram.hpp>

#define VERSION "1.1.0"
/*
 * Changelog:
 * 1.1.0 - 29.11.2020 - ralf.stemmer@offis.de: optional output printed to stderr
 * 1.0.0 - 05.06.2020 - ralf.stemmer@offis.de: initial version (split from pdfplot)
 */

void PrintHelp()
{
    std::cout << "pdfcompare histogram1 histogram2\n\n";
    std::cout << "\tCompares two histograms using Bhattacharyya Distance\n";
}



int main(int argc, char *argv[])
{
    // Check Command Line Arguments
    if(argc == 2)
    {
        std::string argv1(argv[1]); 
        if(argv1 == "--help" or argv1 == "-h")
        {
            PrintHelp();
            return EXIT_SUCCESS;
        }
        else if(argv1 == "--version" or argv1 == "-v")
        {
            std::cout << "pdfcompare " << VERSION << "\n";
            return EXIT_SUCCESS;
        }
    }

    if(argc != 3)
    {
        std::cerr << "\e[1;31mERROR: Expecting exact 2 command line argument. Given were " << argc-1 << "!\e[0m\n";
        PrintHelp();
        return EXIT_FAILURE;
    }

    std::string histogrampath1(argv[1]);
    std::string histogrampath2(argv[2]);


    // Load Histogram Configurations
    Histogram histogram1(histogrampath1);
    Histogram histogram2(histogrampath2);


    // Process Data
    std::cerr << "\e[1;36m • \e[1;34mProcessing samples\e[0m\n";

    histogram1.ProcessSamples();
    histogram2.ProcessSamples();


    // Construct bins
    auto bindef1 = histogram1.DefineBins(20);
    auto bindef2 = histogram2.DefineBins(20);
    BinDefinition commonbins;

    if(bindef1.from < bindef2.from)
        commonbins.from = bindef1.from;
    else
        commonbins.from = bindef2.from;

    if(bindef1.to > bindef2.to)
        commonbins.to = bindef1.to;
    else
        commonbins.to = bindef2.to;

    if(bindef1.size < bindef2.size)
        commonbins.size = bindef1.size;
    else
        commonbins.size = bindef2.size;


    // Align Histograms
    std::cerr << "\e[1;36m • \e[1;34mAligning Histograms\e[0m\n";
    histogram1.RedefineBins(commonbins);
    histogram2.RedefineBins(commonbins);


    // Create Histograms
    std::cerr << "\e[1;36m • \e[1;34mCreating Histograms\e[0m\n";
    histogram1.CreateHistogram();
    histogram2.CreateHistogram();


    // Compare Histograms
    double bhattacharyya = histogram1.BhattacharyyaDistance(histogram2);
    std::cerr << "\n\e[1;35mBhattacharyya Distance: \e[1;37m";
    std::cout << bhattacharyya;
    std::cout << "\n";

    return EXIT_SUCCESS;
}

