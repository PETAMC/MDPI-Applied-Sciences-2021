
#define REPORT_DEFINE_GLOBALS

#include <tlm.h>
#include <systemc.h>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>

#include <vector>
#include <string>
#include <fstream>
#include <random>

#include <hardware/tile.hpp>
#include <hardware/memory.hpp>
#include <hardware/fcfsinterconnect.hpp>
#include <hardware/axiinterconnect.hpp>
#include <software/channel.hpp>
#include <delayvector.hpp>

#include <setup/pythonwrapper.hpp>
#include <setup/experiment.hpp>

// Based on low level measurements.
sc_core::sc_time readdelay(12, sc_core::SC_NS); // \_ per token
sc_core::sc_time writedelay(9, sc_core::SC_NS); // /
// These are the delays for read (lwi) and write (swi) instructions
// including execution and memory access phases.
//
// The cycle accurate model expects exact these values and extrapolates
// for multiple contender C by assuming 
//      readdelay  · (C + 1)    and
//      writedelay · (C + 1)
// The +1 is the initiator of the communication.
// This is a very pessimistic approach and cannot be confirmed by experiments
// For example write access with 6 writing contender was measured
// with 26 cycles, instead of calculated 9·(6+1)=63 cycles
//
// The SystemC event based model takes less care about contention.
// Processing elements in polling mode are not considered as a contender.
// This leads to over optimistic results for many processing elements.

#include <monitor.hpp>
#include <sdfg/sobel2.hpp>
#include <sdfg/jpeg.hpp>

void PrintUsage()
{
    cerr << "--iterations   -i    - Define number of iterations to simulate (default: 1000000)\n";
    cerr << "--skip         -s    - Define number of iterations to skip in the simulation (default: 0)\n";
    cerr << "--experiment   -e    - Select the experiment that shall be simulated (mandatory parameter)\n";
}


void CancelSimulation(sig_atomic_t s)
{
    PythonWrapper &pythonwrapper = PythonWrapper::GetInstance();
    pythonwrapper.ForceShutdown();
    Python &python = Python::GetInstance();
    python.ForceShutdown();
    exit(1); 
}


int sc_main(int argc, char *argv[])
{
    std::srand(0); // 0 is the seed - this is not very random but OK in this case

    // Read command line parameters
    unsigned int maxiterations = 1000000;
    unsigned int skipsamples   = 0;
    std::string  experimentname= "null";
    DISTRIBUTION distribution  = DISTRIBUTION::INJECTED;
    COMMUNICATIONMODEL communicationmodel = COMMUNICATIONMODEL::CYCLEACCURATE;
    bool datadependentdelay    = false;
    bool         functional    = false;
    const char*  tracepath     = nullptr;

    for(int i=0; i<argc; i++)
    {
        if((strncmp("--help", argv[i], 20) == 0) || (strncmp("-h", argv[i], 20) == 0))
        {
            PrintUsage();
            exit(EXIT_SUCCESS);
        }
        if((strncmp("--iterations", argv[i], 20) == 0) || (strncmp("-i", argv[i], 20) == 0))
        {
            i++;
            if(i >= argc)
            {
                cerr << "Invalid use of --iteration. Argument expected!\n";
                PrintUsage();
                exit(EXIT_FAILURE);
            }
            maxiterations = stol(std::string(argv[i]));
            cerr << "\e[1;33mLimiting iterations to " << maxiterations << "\e[0m\n";
        }
        if((strncmp("--skip", argv[i], 20) == 0) || (strncmp("-s", argv[i], 20) == 0))
        {
            i++;
            if(i >= argc)
            {
                cerr << "Invalid use of --skip. Argument expected!\n";
                PrintUsage();
                exit(EXIT_FAILURE);
            }
            skipsamples = stol(std::string(argv[i]));
            cerr << "\e[1;33mSkipping " << skipsamples << " samples\e[0m\n";
        }
        if((strncmp("--experiment", argv[i], 20) == 0) || (strncmp("-e", argv[i], 20) == 0))
        {
            i++;
            if(i >= argc)
            {
                cerr << "Invalid use of --experiment. Argument expected!\n";
                PrintUsage();
                exit(EXIT_FAILURE);
            }
            experimentname = std::string(argv[i]);
            cerr << "\e[1;37mRunning experiment " << experimentname << "\e[0m\n";
        }
        if((strncmp("--functional", argv[i], 20) == 0) || (strncmp("-f", argv[i], 20) == 0))
        {
            std::cerr << "\e[1;31mDEPRECATED: \e[0mUse attribute inside the XML file that describes the experiment!\n";
            functional = true;
            cerr << "\e[1;33mRunning functional test\e[0m\n";
        }
        if((strncmp("--communicationmodel", argv[i], 20) == 0) || (strncmp("-c", argv[i], 20) == 0))
        {
            std::cerr << "\e[1;31mDEPRECATED: \e[0mUse the XML file that describes the experiment!\n";
            i++;
            if(i >= argc)
            {
                cerr << "Invalid use of --communicationmodel. Argument expected!\n";
                PrintUsage();
                exit(EXIT_FAILURE);
            }

            auto modelname = std::string(argv[i]);
            
            if(modelname == "cycleaccurate")
                communicationmodel = COMMUNICATIONMODEL::CYCLEACCURATE;
            else if(modelname == "systemcevents")
                communicationmodel = COMMUNICATIONMODEL::SYSTEMCEVENTS;
            else
            {
                cerr << "\e[1;31mERROR\e[0m Communication Model " << modelname << " not known.\e[0m\n";
                PrintUsage();
                exit(EXIT_FAILURE);
            }

            cerr << "\e[1;34mUsing communication model " << modelname << "\e[0m\n";
        }
        if((strncmp("--distribution", argv[i], 20) == 0) || (strncmp("-d", argv[i], 20) == 0))
        {
            std::cerr << "\e[1;31mDEPRECATED: \e[0mUse the XML file that describes the experiment!\n";
            i++;
            if(i >= argc)
            {
                cerr << "Invalid use of --distribution. Argument expected!\n";
                PrintUsage();
                exit(EXIT_FAILURE);
            }

            auto distributionname = std::string(argv[i]);
            
            if(distributionname == "injected")
                distribution  = DISTRIBUTION::INJECTED;
            else if(distributionname == "gaussian")
                distribution  = DISTRIBUTION::GAUSSIAN;
            else if(distributionname == "uniform")
                distribution  = DISTRIBUTION::UNIFORM;
            else if(distributionname == "wcet")
                distribution  = DISTRIBUTION::WCET;
            else if(distributionname == "gaussian_kde")
                distribution  = DISTRIBUTION::GAUSSIAN_KDE;
            else if(distributionname == "explicit")
                datadependentdelay = true;
            else
            {
                cerr << "\e[1;31mERROR\e[0m Distribution " << distributionname << " not known.\e[0m\n";
                PrintUsage();
                exit(EXIT_FAILURE);
            }

            cerr << "\e[1;34mUsing distribution " << distributionname << "\e[0m\n";
        }
        if((strncmp("--trace", argv[i], 20) == 0) || (strncmp("-t", argv[i], 20) == 0))
        {
            i++;
            if(i >= argc)
            {
                cerr << "Invalid use of --trace. Argument expected!\n";
                PrintUsage();
                exit(EXIT_FAILURE);
            }

            tracepath = argv[i];
            
            cerr << "\e[1;34mWriting trace into " << tracepath << "\e[0m\n";
        }
    }

    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = CancelSimulation;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    cerr << "\e[1;34mPreparing experiment...\e[0m\n";

    // Start embedded python
    PythonWrapper &pythonwrapper = PythonWrapper::GetInstance();

    // Open Experiment
    std::string experimentpath;
    experimentpath  = "./experiments/";
    experimentpath += experimentname;
    experimentpath += ".xml";
    Experiment experiment(experimentpath);

    // Setup models
    Monitor monitor;

    try
    {
        std::tie(distribution, functional, communicationmodel) = experiment.LoadModels();
    }
    catch(const std::runtime_error &e)
    {
        std::cerr << "\e[1;33mLoading model selection from XML file failed. Using settings from command line instead.\n";
    }

    if(communicationmodel == COMMUNICATIONMODEL::MESSAGELEVEL)
    {
        readdelay  = sc_core::SC_ZERO_TIME;
        writedelay = sc_core::SC_ZERO_TIME;
    }

    SharedMemory sharedmemory("SharedMemory", 0x00010000, 32*1024,
            readdelay, writedelay, monitor);
    Interconnect *bus;

    if(communicationmodel == COMMUNICATIONMODEL::CYCLEACCURATE
    or communicationmodel == COMMUNICATIONMODEL::SYSTEMCEVENTS)
    {
        bus = new AXIInterconnect("AXIBus");
    }
    else if(communicationmodel == COMMUNICATIONMODEL::MESSAGELEVEL)
    {
        bus = new FCFSInterconnect("FCFSBus");
    }

    // Present selected models
    std::cerr << "\e[1;36mComputation Model:   \e[1;37m" << distribution;
    if(functional)
        std::cerr << " \e[1;30m(functional)";
    else
        std::cerr << " \e[1;30m(non-functional)";
    if(skipsamples > 0)
        std::cerr << " \e[0;36mskipping first " << skipsamples << " samples";
    std::cerr << "\n";
    std::cerr << "\e[1;36mCommunication Model: \e[1;37m" << communicationmodel << "\n";
    std::cerr << "\e[1;36mShared Memory:       \e[1;37mread = " << readdelay << "\e[1;30m;\e[1;37m write = " << writedelay << "\n";
    std::cerr << "\e[1;36mInterconnect:        \e[1;37mFCFS\n";


    // Create Actors
    monitor.EnableAppOutput(functional);
    monitor.EnableDurationOutput(!functional);
    monitor.EnableTraceOutput(tracepath);

    // Create Channels
    // name, producerate, consumerate, size
    Channel ch_gx      ("ch_gx",    81, 81, 81, monitor, communicationmodel);
    Channel ch_gy      ("ch_gy",    81, 81, 81, monitor, communicationmodel);
    Channel ch_xa      ("ch_xa",     1 , 1,  1, monitor, communicationmodel);
    Channel ch_ya      ("ch_ya",     1 , 1,  1, monitor, communicationmodel);
    Channel ch_pos     ("ch_pos",    2,  2,  2, monitor, communicationmodel);
    Channel ch_gx2     ("ch_gx2",    9,  9,  9, monitor, communicationmodel);
    Channel ch_gy2     ("ch_gy2",    9,  9,  9, monitor, communicationmodel);
    Channel ch_xa2     ("ch_xa2",    1 , 1,  1, monitor, communicationmodel);
    Channel ch_ya2     ("ch_ya2",    1 , 1,  1, monitor, communicationmodel);
    Channel ch_dcoffset("dcoffset",  3,  3,  3, monitor, communicationmodel);
    Channel ch_ency    ("ency",     64, 64, 64, monitor, communicationmodel);
    Channel ch_enccr   ("enccr",    64, 64, 64, monitor, communicationmodel);
    Channel ch_enccb   ("enccb",    64, 64, 64, monitor, communicationmodel);
    Channel ch_prepy   ("prepy",    64, 64, 64, monitor, communicationmodel);
    Channel ch_prepcr  ("prepcr",   64, 64, 64, monitor, communicationmodel);
    Channel ch_prepcb  ("prepcb",   64, 64, 64, monitor, communicationmodel);
    Channel ch_y       ("y",        64, 64, 64, monitor, communicationmodel);
    Channel ch_cr      ("cr",       64, 64, 64, monitor, communicationmodel);
    Channel ch_cb      ("cb",       64, 64, 64, monitor, communicationmodel);

    // Create Delay Vectors
    // Sobel2 Timings
#define SOBEL2_DelayVector(a) DelayVector(sobel2directory + a , distribution, skipsamples)
    std::string sobel2directory = "../PlatformV2/timings/sobel2/bram/";
    DelayVectorMap getpixel2_delay;
    DelayVectorMap gx2_delay;
    DelayVectorMap gy2_delay;
    DelayVectorMap abs2_delay;

    getpixel2_delay["none"] = new SOBEL2_DelayVector("GetPixel.txt");
    getpixel2_delay["ea"]   = new SOBEL2_DelayVector("GetPixel-ea.txt");
    getpixel2_delay["ef"]   = new SOBEL2_DelayVector("GetPixel-ef.txt");
    gx2_delay["none"]       = new SOBEL2_DelayVector("GX.txt");
    gx2_delay["ea"]         = new SOBEL2_DelayVector("GX-ea.txt");
    gx2_delay["ef"]         = new SOBEL2_DelayVector("GX-ef.txt");
    gy2_delay["none"]       = new SOBEL2_DelayVector("GY.txt");
    gy2_delay["ea"]         = new SOBEL2_DelayVector("GY-ea.txt");
    gy2_delay["ef"]         = new SOBEL2_DelayVector("GY-ef.txt");
    abs2_delay["none"]      = new SOBEL2_DelayVector("ABS.txt");
    abs2_delay["ea"]        = new SOBEL2_DelayVector("ABS-ea.txt");
    abs2_delay["ef"]        = new SOBEL2_DelayVector("ABS-ef.txt");
#undef SOBEL2_DelayVector

    // JPEG Timings
#define JPEG_DelayVector(a) DelayVector(jpegdirectory + a , distribution, skipsamples)
    std::string jpegdirectory = "../PlatformV2/timings/jpeg/bram/";
    DelayVectorMap getencodedimageblock_delay;
    DelayVectorMap iq_y_delay;
    DelayVectorMap iq_cr_delay;
    DelayVectorMap iq_cb_delay;
    DelayVectorMap idct_y_delay;
    DelayVectorMap idct_cr_delay;
    DelayVectorMap idct_cb_delay;
    DelayVectorMap creatergbpixels_delay;

    getencodedimageblock_delay["none"] = new JPEG_DelayVector("GetEncodedImageBlock.txt");
    getencodedimageblock_delay["ea"]   = new JPEG_DelayVector("GetEncodedImageBlock-ea.txt");
    getencodedimageblock_delay["ef"]   = new JPEG_DelayVector("GetEncodedImageBlock-ef.txt");

    iq_y_delay["none"]                 = new JPEG_DelayVector("InverseQuantization_Y.txt");
    iq_y_delay["ea"]                   = new JPEG_DelayVector("InverseQuantization_Y-ea.txt");
    iq_y_delay["ef"]                   = new JPEG_DelayVector("InverseQuantization_Y-ef.txt");
    iq_cr_delay["none"]                = new JPEG_DelayVector("InverseQuantization_Cr.txt");
    iq_cr_delay["ea"]                  = new JPEG_DelayVector("InverseQuantization_Cr-ea.txt");
    iq_cr_delay["ef"]                  = new JPEG_DelayVector("InverseQuantization_Cr-ef.txt");
    iq_cb_delay["none"]                = new JPEG_DelayVector("InverseQuantization_Cb.txt");
    iq_cb_delay["ea"]                  = new JPEG_DelayVector("InverseQuantization_Cb-ea.txt");
    iq_cb_delay["ef"]                  = new JPEG_DelayVector("InverseQuantization_Cb-ef.txt");

    idct_y_delay["none"]               = new JPEG_DelayVector("IDCT_Y.txt");
    idct_y_delay["ea"]                 = new JPEG_DelayVector("IDCT_Y-ea.txt");
    idct_y_delay["ef"]                 = new JPEG_DelayVector("IDCT_Y-ef.txt");
    idct_cr_delay["none"]              = new JPEG_DelayVector("IDCT_Cr.txt");
    idct_cr_delay["ea"]                = new JPEG_DelayVector("IDCT_Cr-ea.txt");
    idct_cr_delay["ef"]                = new JPEG_DelayVector("IDCT_Cr-ef.txt");
    idct_cb_delay["none"]              = new JPEG_DelayVector("IDCT_Cb.txt");
    idct_cb_delay["ea"]                = new JPEG_DelayVector("IDCT_Cb-ea.txt");
    idct_cb_delay["ef"]                = new JPEG_DelayVector("IDCT_Cb-ef.txt");

    creatergbpixels_delay["none"]      = new JPEG_DelayVector("CreateRGBPixels.txt");
    creatergbpixels_delay["ea"]        = new JPEG_DelayVector("CreateRGBPixels-ea.txt");
    creatergbpixels_delay["ef"]        = new JPEG_DelayVector("CreateRGBPixels-ef.txt");
#undef JPEG_DelayVector



    // Create Architecture Components
    Tile mb0("MB0", maxiterations, monitor);
    Tile mb1("MB1", maxiterations, monitor);
    Tile mb2("MB2", maxiterations, monitor);
    Tile mb3("MB3", maxiterations, monitor);
    Tile mb4("MB4", maxiterations, monitor);
    Tile mb5("MB5", maxiterations, monitor);
    Tile mb6("MB6", maxiterations, monitor);


    // Create & Load Actors

    // Load application
    bool success;
    SDFApplication application;
    success = experiment.LoadApplication(&application);
    if(not success)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading SDF Application for experiment "
                  << experimentname
                  << " failed!\n";
        exit(EXIT_FAILURE);
    }


    // Type          Name       Name        Delay Vector     Monitor  Application
    Sobel2::GetPixel getpixel2("GetPixel2", getpixel2_delay, monitor, application, datadependentdelay);
    Sobel2::GX       gx2      ("GX2"      , gx2_delay      , monitor, application, datadependentdelay);
    Sobel2::GY       gy2      ("GY2"      , gy2_delay      , monitor, application, datadependentdelay);
    Sobel2::ABS      abs2     ("ABS2"     , abs2_delay     , monitor, application, datadependentdelay);

    // Type              Name                   Name                  Delay Vector                Monitor
    JPEG::GetEncodedImageBlock getencodedimageblock("GetEncodedImageBlock", getencodedimageblock_delay, monitor, application);
    JPEG::IQ_Y                 iq_y                ("IQ_Y"                , iq_y_delay                , monitor, application);
    JPEG::IQ_Cr                iq_cr               ("IQ_Cr"               , iq_cr_delay               , monitor, application);
    JPEG::IQ_Cb                iq_cb               ("IQ_Cb"               , iq_cb_delay               , monitor, application);
    JPEG::IDCT_Y               idct_y              ("IDCT_Y"              , idct_y_delay              , monitor, application);
    JPEG::IDCT_Cr              idct_cr             ("IDCT_Cr"             , idct_cr_delay             , monitor, application);
    JPEG::IDCT_Cb              idct_cb             ("IDCT_Cb"             , idct_cb_delay             , monitor, application);
    JPEG::CreateRGBPixels      creatergbpixels     ("CreatetRGBPixels"    , creatergbpixels_delay     , monitor, application);

    // Define start and finish actors
    getpixel2.DefineAsStartActor();
    abs2.DefineAsFinishActor();

    getencodedimageblock.DefineAsStartActor();
    creatergbpixels.DefineAsFinishActor();

    // TODO: (work in progress) Load Dynamic Experiment
    TileMap tilemap;
    tilemap["MB0"] = &mb0;
    tilemap["MB1"] = &mb1;
    tilemap["MB2"] = &mb2;
    tilemap["MB3"] = &mb3;
    tilemap["MB4"] = &mb4;
    tilemap["MB5"] = &mb5;
    tilemap["MB6"] = &mb6;

    ActorMap actormap;
    actormap["GetPixel2"           ] = &getpixel2           ;
    actormap["GX2"                 ] = &gx2                 ;
    actormap["GY2"                 ] = &gy2                 ;
    actormap["ABS2"                ] = &abs2                ;
    actormap["GetEncodedImageBlock"] = &getencodedimageblock;
    actormap["IQ_Y"                ] = &iq_y                ;
    actormap["IQ_Cr"               ] = &iq_cr               ;
    actormap["IQ_Cb"               ] = &iq_cb               ;
    actormap["IDCT_Y"              ] = &idct_y              ;
    actormap["IDCT_Cr"             ] = &idct_cr             ;
    actormap["IDCT_Cb"             ] = &idct_cb             ;
    actormap["CreateRGBPixels"     ] = &creatergbpixels     ;
    
    ChannelMap channelmap;
    channelmap["ch_pos"     ] = &ch_pos     ;
    channelmap["ch_gx2"     ] = &ch_gx2     ;
    channelmap["ch_gy2"     ] = &ch_gy2     ;
    channelmap["ch_xa2"     ] = &ch_xa2     ;
    channelmap["ch_ya2"     ] = &ch_ya2     ;
    channelmap["ch_dcoffset"] = &ch_dcoffset;
    channelmap["ch_ency"    ] = &ch_ency    ;
    channelmap["ch_enccr"   ] = &ch_enccr   ;
    channelmap["ch_enccb"   ] = &ch_enccb   ;
    channelmap["ch_prepy"   ] = &ch_prepy   ;
    channelmap["ch_prepcr"  ] = &ch_prepcr  ;
    channelmap["ch_prepcb"  ] = &ch_prepcb  ;
    channelmap["ch_y"       ] = &ch_y       ;
    channelmap["ch_cr"      ] = &ch_cr      ;
    channelmap["ch_cb"      ] = &ch_cb      ;
    
    MemoryMap memorymap;
    memorymap["SharedMemory"] = &sharedmemory;


    // Load mappings
    success = experiment.LoadActorMapping(tilemap, actormap);
    if(not success)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading actor mapping for experiment "
                  << experimentname
                  << " failed!\n";
        exit(EXIT_FAILURE);
    }

    success = experiment.LoadChannelMapping(memorymap, tilemap, channelmap);
    if(not success)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading channel mapping for  experiment "
                  << experimentname
                  << " failed!\n";
        exit(EXIT_FAILURE);
    }


    // Connection of actors
    getpixel2 << ch_pos;
    getpixel2 >> ch_gx2;
    getpixel2 >> ch_gy2;
    getpixel2 >> ch_pos;
    gx2       << ch_gx2;
    gx2       >> ch_xa2;
    gy2       << ch_gy2;
    gy2       >> ch_ya2;
    abs2      << ch_xa2;
    abs2      << ch_ya2;

    getencodedimageblock << ch_dcoffset;
    getencodedimageblock >> ch_ency;
    getencodedimageblock >> ch_enccr;
    getencodedimageblock >> ch_enccb;
    getencodedimageblock >> ch_dcoffset;

    iq_y    << ch_ency;
    iq_y    >> ch_prepy;
    iq_cr   << ch_enccr;
    iq_cr   >> ch_prepcr;
    iq_cb   << ch_enccb;
    iq_cb   >> ch_prepcb;

    idct_y  << ch_prepy;
    idct_y  >> ch_y;
    idct_cr << ch_prepcr;
    idct_cr >> ch_cr;
    idct_cb << ch_prepcb;
    idct_cb >> ch_cb;

    creatergbpixels << ch_y;
    creatergbpixels << ch_cr;
    creatergbpixels << ch_cb;

    // Build Architecture
    *bus << mb0;
    *bus << mb1;
    *bus << mb2;
    *bus << mb3;
    *bus << mb4;
    *bus << mb5;
    *bus << mb6;
    *bus << sharedmemory;


    // Start simulation
    std::cerr << "\e[1;37mSimulation started\n\e[0m";
    sc_core::sc_start();

    std::cerr << "\e[1;37mSimulation ended\n\e[0m";

    delete bus;

    pythonwrapper.ForceShutdown();
    Python &python = Python::GetInstance();
    python.ForceShutdown();

    return 0;
}
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

