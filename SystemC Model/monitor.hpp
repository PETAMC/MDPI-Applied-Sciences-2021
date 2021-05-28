#ifndef MONITOR_HPP
#define MONITOR_HPP

#include <map>
#include <string>
#include <systemc>
#ifdef ENABLE_EVD
#include <evdgen.hpp>
#endif

class Monitor
{
    public:
        Monitor();
        ~Monitor();

        void IterationBegin();
        void IterationEnd();

        void EnableDurationOutput(bool enable=true);
        void EnableAppOutput(bool enable=true);
        
        void EnableTraceOutput(const char* tracepath=nullptr);
        void ExpandTrace(std::string signalname, std::string value);

        void PrintAppOutput(std::string output);

    private:
        bool enableappoutput;
        bool enabledurationoutput;
        bool enabletraceoutput;
        unsigned int iterationstarted;
        unsigned int iterationended;
        std::map<unsigned int, sc_core::sc_time> iterationstarts;

#ifdef ENABLE_EVD
        EventDumpGenerator evd;
#endif
};


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

