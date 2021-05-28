#include <iostream>
#include <vector>
#include <limits>
#include <cstdlib>
#include <libconfig.h++>
#include <histogram.hpp>
#include <plot.hpp>

#define VERSION "2.4.1"
/*
 * Changelog:
 * 2.4.1 - 03.12.2020 - ralf.stemmer@offis.de:
 *                              - Minor bin detection improvement
 *                              - Text alignment in legend improved
 * 2.4.0 - 03.12.2020 - ralf.stemmer@offis.de:
 *                              - Cut off bins with a probability double as high as the second highest bin
 * 2.3.0 - 29.11.2020 - ralf.stemmer@offis.de:
 *                              - fixed compiler errors (how did they appear?)
 *                              - Force bin-size to be at least 1
 *                              - Force bin-size to be at least max for none-distributions
 * 2.2.1 - 04.06.2020 - ralf.stemmer@offis.de: pdfcompare2 binary is now called pdfplot
 * 2.2.0 - 16.04.2020 - ralf.stemmer@offis.de:
 *                              - Configuration for legend positioning added
 * 2.1.0 - 16.04.2020 - ralf.stemmer@offis.de:
 *                              - Allow variable height and width of the graph
 *                              - Assumes default settings for incomplete configurations
 * 2.0.2 - 03.04.2020 - ralf.stemmer@offis.de: Allow samples in scientific notation ("x.ye+z")
 * 2.0.1 - 03.04.2020 - ralf.stemmer@offis.de: Update of Y-Axis tick optimization algorithm
 */

void PrintHelp()
{
    std::cout << "pdfplot configfile outputfile\n\n";
    std::cout << "\tconfigfile \t Defines the plot. See example plot.cfg\n";
    std::cout << "\toutputfile \t Path to a pdf file to render the plot in\n";
}

void LoadStringOrExit(const libconfig::Setting &setting, const char *name, std::string &value, const char *fallback=nullptr)
{
    if(not setting.exists(name))
    {
        if(fallback != nullptr)
        {
            value = fallback;
            return;
        }
        std::cerr << "\e[1;31mERROR: Missing configuration \"" << name << "\"\e[0m\n";
        exit(EXIT_FAILURE);
    }
    if(not setting.lookupValue(name, value))
    {
        std::cerr << "\e[1;31mERROR: Failed reading configuration \"" << name << "\" (string expected)\e[0m\n";
        exit(EXIT_FAILURE);
    }
    return;
}
void LoadBooleanOrExit(const libconfig::Setting &setting, const char *name, bool &value, const bool fallback)
{
    if(not setting.exists(name))
    {
        value = fallback;
        return;
    }
    if(not setting.lookupValue(name, value))
    {
        std::cerr << "\e[1;31mERROR: Failed reading configuration \"" << name << "\" (boolean expected)\e[0m\n";
        exit(EXIT_FAILURE);
    }
    return;
}
void LoadDoubleOrExit(const libconfig::Setting &setting, const char *name, double &value, const double fallback)
{
    if(not setting.exists(name))
    {
        value = fallback;
        return;
    }
    if(not setting.lookupValue(name, value))
    {
        std::cerr << "\e[1;31mERROR: Failed reading configuration \"" << name << "\" (double expected)\e[0m\n";
        exit(EXIT_FAILURE);
    }
    return;
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
            std::cout << "pdfplot " << VERSION << "\n";
            return EXIT_SUCCESS;
        }
    }

    if(argc != 3)
    {
        std::cerr << "\e[1;31mERROR: Expecting exact 2 command line argument. Given were " << argc-1 << "!\e[0m\n";
        PrintHelp();
        return EXIT_FAILURE;
    }

    std::string configpath(argv[1]);
    std::string outputpath(argv[2]);

    // Open Configuration
    libconfig::Config cfg;

    std::cout << "\e[1;36m • \e[1;34mLoading configuration\e[0m\n";
    try
    {
        cfg.readFile(configpath.c_str());
    }
    catch(const libconfig::FileIOException &fioex)
    {
        std::cerr << "\e[1;31mERROR: I/O error while reading configuration file \""
            << configpath << "\"\e[0m\n";
        return EXIT_FAILURE;
    }
    catch(const libconfig::ParseException &pex)
    {
        std::cerr << "\e[1;31mERROR: Configuration parse error at \""
            << pex.getFile() << "\":"
            << pex.getLine() << " - "
            << pex.getError() << "\e[0m\n";
        return EXIT_FAILURE;
    }

    
    // Load Histogram Configurations
    const libconfig::Setting &cfgroot = cfg.getRoot();
    if(not cfgroot.exists("histograms"))
    {
        std::cerr << "\e[1;31mERROR: No histograms-list inside configuration \"" << configpath << "\"\e[0m\n";
        exit(EXIT_FAILURE);
    }

    std::vector<Histogram>    histograms;
    const libconfig::Setting &histogramdescriptions = cfgroot["histograms"];

    // Load Plot Configuration
    std::string xaxislabel;
    std::string yaxislabel;
    std::string legendpos;
    bool        alignbins;
    bool        drawmax;
    bool        drawavg;
    double      pagewidth;
    double      pageheight;

    LoadStringOrExit (cfgroot, "xaxislabel",  xaxislabel,   "Probability");
    LoadStringOrExit (cfgroot, "yaxislabel",  yaxislabel,   "Values");
    LoadStringOrExit (cfgroot, "legendpos",   legendpos,    "left");
    LoadBooleanOrExit(cfgroot, "alignbins",   alignbins,    false);
    LoadBooleanOrExit(cfgroot, "drawmax",     drawmax,      false);
    LoadBooleanOrExit(cfgroot, "drawavg",     drawavg,      false);
    LoadDoubleOrExit (cfgroot, "pagewidth",   pagewidth,    13.5);
    LoadDoubleOrExit (cfgroot, "pageheight",  pageheight,   8.0);

    for(int i=0; i<histogramdescriptions.getLength(); i++)
    {
        const libconfig::Setting &hd = histogramdescriptions[i];

        std::string path;
        std::string label;
        std::string color;

        LoadStringOrExit(hd, "data",  path);
        LoadStringOrExit(hd, "label", label,    "Data");
        LoadStringOrExit(hd, "color", color,    "#808080");

        histograms.emplace_back(label, color, path);
    }


    // Process Data
    std::cout << "\e[1;36m • \e[1;34mProcessing samples\e[0m\n";
    long firstvalue = std::numeric_limits<long>::max();
    long lastvalue  = std::numeric_limits<long>::min();
    long smalestbin = std::numeric_limits<long>::max();

    for(Histogram &histogram : histograms)
    {
        histogram.ProcessSamples();
        auto &bindef = histogram.DefineBins(20);
        
        if(firstvalue > bindef.from)
            firstvalue = bindef.from;
        if(lastvalue < bindef.to)
            lastvalue = bindef.to;
        if(smalestbin > bindef.size)
            smalestbin = bindef.size;
    }


    // Align Histograms
    if(alignbins)
    {
        std::cout << "\e[1;36m • \e[1;34mAligning Histograms\e[0m\n";
        BinDefinition commonbindef = {.from=firstvalue, .to=lastvalue, .size=smalestbin};
        for(Histogram &histogram : histograms)
        {
            histogram.RedefineBins(commonbindef);
        }
    }


    // Create Histograms
    std::cout << "\e[1;36m • \e[1;34mCreating Histograms\e[0m\n";
    for(Histogram &histogram : histograms)
    {
        histogram.CreateHistogram();
    }


    // Render Plot
    std::cout << "\e[1;36m • \e[1;34mRendering plot\e[0m\n";
    //Plot plot(outputpath, 13.5, 8.0);
    Plot plot(outputpath, pagewidth, pageheight);
    plot.SetLabels(xaxislabel, yaxislabel);
    plot.SetMaximumMarker(drawmax);
    plot.SetAverageMarker(drawavg);
    plot.SetLegendVAlignment(legendpos);
    plot.Draw(histograms);

    return EXIT_SUCCESS;
}

