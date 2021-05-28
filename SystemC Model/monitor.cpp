#include <monitor.hpp>

Monitor::Monitor()
    : iterationstarted(0)
    , iterationended(0)
    , enableappoutput(false)
    , enabledurationoutput(true)
    , enabletraceoutput(false)
#ifdef ENABLE_EVD
    , evd()
#endif
{
}

Monitor::~Monitor()
{
}



void Monitor::EnableDurationOutput(bool enable)
{
    this->enabledurationoutput = enable;
}
void Monitor::EnableAppOutput(bool enable)
{
    this->enableappoutput = enable;
}
void Monitor::EnableTraceOutput(const char* tracepath)
{
    if(tracepath == nullptr)
    {
        this->enabletraceoutput = false;
    }
    else
    {
#ifdef ENABLE_EVD
        this->evd.Open(tracepath);
        this->enabletraceoutput = true;
#else
        std::cerr << "\e[1;31mERROR: This binary was compiled without libevd. \e[1;30m(Tracing stays disabled).\n";
#endif
    }
}



void Monitor::ExpandTrace(std::string signalname, std::string value)
{
    if(this->enabletraceoutput)
    {
#ifdef ENABLE_EVD
        sc_core::sc_time timestamp = sc_core::sc_time_stamp();
        this->evd.AddEvent(signalname, timestamp.value()/1000, value);
#endif
    }
}



void Monitor::PrintAppOutput(std::string output)
{
    if(this->enableappoutput == true)
        std::cout << output;
}



void Monitor::IterationBegin()
{
    this->iterationstarted++;
    this->iterationstarts.emplace(iterationstarted, sc_core::sc_time_stamp());
}



void Monitor::IterationEnd()
{
    // Get information about start time of the currently ended iteration
    this->iterationended++;

    auto startinfos = this->iterationstarts.find(this->iterationended);
    if(startinfos == this->iterationstarts.end())
    {
        std::cerr << "\e[1;31mERROR: \e[0mCannot find start time stamp for a finished iteration.\n";
        return;
    }

    // Calculate time difference (= iteration duration)
    sc_core::sc_time starttime;
    sc_core::sc_time stoptime;
    sc_core::sc_time duration;

    starttime = startinfos->second;
    stoptime  = sc_core::sc_time_stamp();
    duration  = stoptime - starttime;

    if(this->enabledurationoutput == true)
        std::cout << std::dec << duration.value() / 1000 << "\n";
}



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

