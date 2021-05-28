#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <systemc>
#include <string>

#include <monitor.hpp>
#include <hardware/interconnect.hpp>
#include <software/sdf.h>

enum COMMUNICATIONMODEL
{
    CYCLEACCURATE,
    SYSTEMCEVENTS,
    MESSAGELEVEL
};

class Channel
    : public sc_core::sc_module
{
    public:
        Channel(std::string name, unsigned int prate, unsigned int crate, unsigned int size, Monitor &monitor, COMMUNICATIONMODEL model = COMMUNICATIONMODEL::CYCLEACCURATE);

        void ChangeProducerTile(Tile *tile);
        void ChangeConsumerTile(Tile *tile);

        std::string name;
        unsigned long long usageaddress; // Virtual address
        unsigned long long indexaddress; // Virtual address - index of the first token in the FIFO
        unsigned long long fifoaddress;  // Virtual address
        unsigned int producerate;
        unsigned int consumerate;

        unsigned int fifosize; // in tokens

        void ReadTokens(token_t tokens[]);
        void WriteTokens(token_t tokens[]);

    private:
        void TracePhase(const char* phase);

        void ReadTokensCycleAccurate(token_t tokens[]);
        void WriteTokensCycleAccurate(token_t tokens[]);

        void ReadTokensEventBased(token_t tokens[]);
        void WriteTokensEventBased(token_t tokens[]);

        void ReadTokensMessageLevel(token_t tokens[]);
        void WriteTokensMessageLevel(token_t tokens[]);
        int CalculateCopyDelay(int numTokens, int delayOffset, int delayThisLoop);

        Tile *consumertile;
        Tile *producertile;

        sc_core::sc_event emptyevent;
        sc_core::sc_event fullevent;

        Monitor *monitor;   // Can be NULL!

        COMMUNICATIONMODEL model;
};


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

