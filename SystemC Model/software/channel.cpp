#include <atomic>

#include <software/channel.hpp>
#include <hardware/tile.hpp>


Channel::Channel(std::string name, unsigned int prate, unsigned int crate, unsigned int size, Monitor &monitor, COMMUNICATIONMODEL model)
            : sc_core::sc_module(sc_core::sc_module_name(name.c_str()))
            , name(name)
            , usageaddress(0)
            , indexaddress(0)
            , fifoaddress(0)
            , producerate(prate)
            , consumerate(crate)
            , fifosize(size)
            , consumertile(NULL)
            , producertile(NULL)
            , monitor(&monitor)
            , model(model)
{
};



void Channel::ChangeConsumerTile(Tile *tile)
{
    if(this->consumertile != NULL)
    {
        std::cerr << "\e[1;33mWARNING:\e[0m Changing associated consumer tile of channel that got already associated to a different tile!\n";
    }
    this->consumertile = tile;
}

void Channel::ChangeProducerTile(Tile *tile)
{
    if(this->producertile != NULL)
    {
        std::cerr << "\e[1;33mWARNING:\e[0m Changing associated producer tile of channel that got already associated to a different tile!\n";
    }
    this->producertile = tile;
}



void Channel::TracePhase(const char* phase)
{
    if(not this->monitor)
        return;

    this->monitor->ExpandTrace(this->name, phase);
}



void Channel::ReadTokens(token_t tokens[])
{
    switch(this->model)
    {
        case COMMUNICATIONMODEL::CYCLEACCURATE:
            this->ReadTokensCycleAccurate(tokens);
            break;

        case COMMUNICATIONMODEL::SYSTEMCEVENTS:
            this->ReadTokensEventBased(tokens);
            break;

        case COMMUNICATIONMODEL::MESSAGELEVEL:
            this->ReadTokensMessageLevel(tokens);
            break;

        default:
            std::cerr << "\e[1;31mERROR:\e[0m Selected communication model for reading tokens not yet implemented!\n";
    }
}



void Channel::WriteTokens(token_t tokens[])
{
    switch(this->model)
    {
        case COMMUNICATIONMODEL::CYCLEACCURATE:
            this->WriteTokensCycleAccurate(tokens);
            break;

        case COMMUNICATIONMODEL::SYSTEMCEVENTS:
            this->WriteTokensEventBased(tokens);
            break;

        case COMMUNICATIONMODEL::MESSAGELEVEL:
            this->WriteTokensMessageLevel(tokens);
            break;

        default:
            std::cerr << "\e[1;31mERROR:\e[0m Selected communication model for writing tokens not yet implemented!\n";
    }
}


// Cycle Accurate

void Channel::ReadTokensCycleAccurate(token_t tokens[])
{
    // Initialization Block
    this->TracePhase("R:init.");
    unsigned int usage;
    sc_core::wait(1, sc_core::SC_NS);

    // Polling Block
    this->TracePhase("R:polling");
    do
    {
        this->consumertile->ReadWord(this->usageaddress, &usage);

        sc_core::wait(1, sc_core::SC_NS);
        if(usage != 0)
            break;
        sc_core::wait(2, sc_core::SC_NS);
    }
    while(true);

    // Preparation Block
    this->TracePhase("R:prep.");
    unsigned int index;
    index = 0;
    sc_core::wait(5, sc_core::SC_NS);

    // Copy Block
    this->TracePhase("R:copying");
    for(unsigned int tokenindex = 0; tokenindex < this->consumerate; tokenindex++)
    {
        unsigned long long address;
        address  = this->fifoaddress;
        address += index;
        this->consumertile->ReadWord(address, reinterpret_cast< unsigned int* >(&tokens[tokenindex]));
        index   += 1;

        sc_core::wait(5, sc_core::SC_NS);
        if(tokenindex+1 < this->consumerate)
            sc_core::wait(2, sc_core::SC_NS);
    }
    sc_core::wait(1, sc_core::SC_NS);

    // Update meta data
    this->TracePhase("R:manag.");
    usage = 0;
    this->consumertile->WriteWord(this->usageaddress, &usage);
    sc_core::wait(3, sc_core::SC_NS);
    this->TracePhase("idle");
}



void Channel::WriteTokensCycleAccurate(token_t tokens[])
{
    // Initialization Block
    this->TracePhase("W:init.");
    unsigned int usage;
    sc_core::wait(1, sc_core::SC_NS);

    // Polling Block
    this->TracePhase("W:polling");
    do
    {
        this->producertile->ReadWord(this->usageaddress, &usage);
        sc_core::wait(1, sc_core::SC_NS);
        if(usage == 0)
            break;
        sc_core::wait(2, sc_core::SC_NS);

    }
    while(true);

    // Preparation Block
    this->TracePhase("W:prep.");
    unsigned int index;
    index = 0;
    sc_core::wait(4, sc_core::SC_NS);

    // Copy Block
    this->TracePhase("W:copying");
    for(unsigned int tokenindex = 0; tokenindex < this->producerate; tokenindex++)
    {
        unsigned long long address;
        address  = this->fifoaddress;
        address += index;
        sc_core::wait(2, sc_core::SC_NS);
        this->producertile->WriteWord(address, reinterpret_cast< unsigned int* >(&tokens[tokenindex]));
        index   += 1;
        sc_core::wait(5, sc_core::SC_NS);
    }
    
    // Management Block
    this->TracePhase("W:manag.");
    usage = 1;
    this->producertile->WriteWord(this->usageaddress, &usage);
    sc_core::wait(3, sc_core::SC_NS);

    this->TracePhase("idle");
}



// Event Based

void Channel::ReadTokensEventBased(token_t tokens[])
{
    // Initialization Block
    this->TracePhase("R:init.");
    unsigned int usage;
    sc_core::wait(1, sc_core::SC_NS);

    // Polling Block
    this->TracePhase("R:polling");
    this->consumertile->ReadWord(this->usageaddress, &usage);
    if(usage == 0)
        sc_core::wait(this->fullevent);

    // Preparation Block
    this->TracePhase("R:prep.");
    unsigned int index;
    index = 0;
    sc_core::wait(5, sc_core::SC_NS);

    // Copy Block
    this->TracePhase("R:copying");
    for(unsigned int tokenindex = 0; tokenindex < this->consumerate; tokenindex++)
    {
        unsigned long long address;
        address  = this->fifoaddress;
        address += index;
        this->consumertile->ReadWord(address, reinterpret_cast< unsigned int* >(&tokens[tokenindex]));
        index   += 1;

        sc_core::wait(5, sc_core::SC_NS);
        if(tokenindex+1 < this->consumerate)
            sc_core::wait(2, sc_core::SC_NS);
    }
    sc_core::wait(1, sc_core::SC_NS);

    // Update meta data
    this->TracePhase("R:manag.");
    usage = 0;
    this->consumertile->WriteWord(this->usageaddress, &usage);
    this->emptyevent.notify();
    sc_core::wait(3, sc_core::SC_NS);
    this->TracePhase("idle");
}



void Channel::WriteTokensEventBased(token_t tokens[])
{
    // Initialization Block
    this->TracePhase("W:init.");
    unsigned int usage;
    sc_core::wait(1, sc_core::SC_NS);

    // Polling Block
    this->TracePhase("W:polling");
    this->producertile->ReadWord(this->usageaddress, &usage);
    if(usage != 0)
        sc_core::wait(this->emptyevent);

    // Preparation Block
    this->TracePhase("W:prep.");
    unsigned int index;
    index = 0;
    sc_core::wait(4, sc_core::SC_NS);

    // Copy Block
    this->TracePhase("W:copying");
    for(unsigned int tokenindex = 0; tokenindex < this->producerate; tokenindex++)
    {
        unsigned long long address;
        address  = this->fifoaddress;
        address += index;
        sc_core::wait(2, sc_core::SC_NS);
        this->producertile->WriteWord(address, reinterpret_cast< unsigned int* >(&tokens[tokenindex]));
        index   += 1;
        sc_core::wait(5, sc_core::SC_NS);
    }
    
    // Management Block
    this->TracePhase("W:manag.");
    usage = 1;
    this->producertile->WriteWord(this->usageaddress, &usage);
    this->fullevent.notify();
    sc_core::wait(3, sc_core::SC_NS);
    this->TracePhase("idle");
}



// Message Level (Works only for shared communication)
struct
{
    const int t_r      =  8;         //read
    const int t_p      =  8;         //polling
    const int t_w      =  5;         //write         
    const int t_rl     = 14;         //wait to the next read
    const int t_wl     = 13;         //wait to the next write
    const int t_pl     =  7;         //wait to the next polling
    const int t_r_loop = t_r + t_rl; //t_r_loop=t_r+t_rl
    const int t_w_loop = t_w + t_wl; //t_w_loop=t_w+t_wl
    const int t_p_loop = t_p + t_pl; //t_p_loop=t_p+t_pl
    const int t_pr_r   = 15;         //t_pre_read
    const int t_po_r   = 11;         //t_post_read
    const int t_pr_w   = 15;         //t_pre_write
    const int t_po_w   =  9;         //t_post_write
    const int t_init_r = 15;         //t_init_read
    const int t_init_w = 16;         //t_init_write
    int DelayOffsetRead(int numTokens)
    {
        return (t_p+t_r*numTokens+t_rl*(numTokens-1)+t_po_r+t_w);
    }

    int DelayOffsetWrite(int numTokens)
    {
        return (t_p+t_w*numTokens+t_wl*(numTokens-1)+t_po_w+t_w);
    }
} ElementaryDelays;

static std::atomic<int> NumPollingActors{0};
static std::atomic<int> NumWritingActors{0};
static std::atomic<int> NumReadingActors{0};

void Channel::ReadTokensMessageLevel(token_t tokens[])
{
    /* There is no reason to distinguish
    if(this->consumertile == this->producertile)
    {
        std::cerr << "\e[1;33mThe Message Level model is only characterized for shared memory communication."
                  << "Yet it is called for a private communication which leads to wrong results!";
    }
    */

    //Initialization
    this->TracePhase("R:init.");
    sc_core::wait(ElementaryDelays.t_init_r, sc_core::SC_NS); 

    //Polling
    this->TracePhase("R:polling");
    unsigned int usage;
    this->consumertile->ReadWord(this->usageaddress, &usage);
    if(usage == 0)
    {
        NumPollingActors++;
        wait(this->fullevent);
        NumPollingActors--;
    }

    //Preparation
    this->TracePhase("R:prep.");
    sc_core::wait(ElementaryDelays.t_pr_r, sc_core::SC_NS); 

    //Copy data
    this->TracePhase("R:copying");
    NumReadingActors++;

    int delayOffset;
    int copydelay;
    delayOffset= ElementaryDelays.DelayOffsetRead(this->consumerate);
    copydelay = this->CalculateCopyDelay(
            this->consumerate,
            delayOffset,
            ElementaryDelays.t_w_loop);
    sc_core::wait(copydelay, sc_core::SC_NS);

    unsigned int index = 0;
    for(unsigned int tokenindex = 0; tokenindex < this->consumerate; tokenindex++)
    {
        unsigned long long address;
        address  = this->fifoaddress;
        address += index;
        this->consumertile->ReadWord(address, reinterpret_cast< unsigned int* >(&tokens[tokenindex]));
        index   += 1;
    }

    this->TracePhase("R:manag.");
    usage = 0;
    this->consumertile->WriteWord(this->usageaddress, &usage);
    this->emptyevent.notify();
    NumReadingActors--;

    this->TracePhase("R:idle");
    return;
}



void Channel::WriteTokensMessageLevel(token_t tokens[])
{
    /* There is no reason to distinguish
    if(this->consumertile == this->producertile)
    {
        std::cerr << "\e[1;33mThe Message Level model is only characterized for shared memory communication."
                  << "Yet it is called for a private communication which leads to wrong results!";
    }
    */

    //Initialization
    this->TracePhase("W:init.");
    sc_core::wait(ElementaryDelays.t_init_w, sc_core::SC_NS);      

    //Polling
    this->TracePhase("W:polling");
    unsigned int usage;
    this->producertile->ReadWord(this->usageaddress, &usage);
    if(usage != 0)
    {
        NumPollingActors++;
        wait(this->emptyevent);
        NumPollingActors--;
    }

    //Preparation
    this->TracePhase("W:prep.");
    sc_core::wait(ElementaryDelays.t_pr_w, sc_core::SC_NS); 

    //Copy data
    this->TracePhase("W:copying");
    NumWritingActors++;

    int delayOffset;
    int copydelay;
    delayOffset= ElementaryDelays.DelayOffsetWrite(this->producerate);
    copydelay = this->CalculateCopyDelay(
            this->producerate,
            delayOffset,
            ElementaryDelays.t_w_loop);
    sc_core::wait(copydelay, sc_core::SC_NS);

    unsigned int index = 0;
    for(unsigned int tokenindex = 0; tokenindex < this->producerate; tokenindex++)
    {
        unsigned long long address;
        address  = this->fifoaddress;
        address += index;
        this->producertile->WriteWord(address, reinterpret_cast< unsigned int* >(&tokens[tokenindex]));
        index   += 1;
    }

    this->TracePhase("W:manag.");
    usage = 1;
    this->producertile->WriteWord(this->usageaddress, &usage);
    this->fullevent.notify();
    NumWritingActors--;

    this->TracePhase("W:idle");
    return;
}



int Channel::CalculateCopyDelay(int numTokens, int delayOffset, int delayThisLoop)
{
    int numActiveActors = NumWritingActors + NumReadingActors + NumPollingActors;
    int delay;
    if(numActiveActors <= 2) // two cores
    {
        int delayAllActiveLoops =
                NumPollingActors * ElementaryDelays.t_p_loop +
                NumWritingActors * ElementaryDelays.t_w_loop +
                NumReadingActors * ElementaryDelays.t_r_loop;

        int x = delayAllActiveLoops - (numActiveActors * delayThisLoop);
        if(x > 0)
            delay = delayOffset + x * (numTokens + 2); // n tokens + 1 token update usage (TODO: +1/+2???)
        else
            delay = delayOffset;

        //if (pCount*t_p_loop+wCount*t_w_loop+rCount*t_r_loop-(pCount+rCount+wCount)*delayThisLoop > 0)  
        //    delay = delayOffset + (pCount*t_p_loop+wCount*t_w_loop+rCount*t_r_loop-(pCount+rCount+wCount)*delayThisLoop)*(numTokens+1); // n tokens + 1 token update usage
        //else
        //    delay = delayOffset;
    }
    else     // n cores (n>2) 
    {
        int delayAllActiveAccesses =
                NumPollingActors * ElementaryDelays.t_p +
                NumWritingActors * ElementaryDelays.t_w +
                NumReadingActors * ElementaryDelays.t_r;

        int y = delayAllActiveAccesses - delayThisLoop;
        if(y > 0)
            delay = delayOffset + y * (numTokens + 2); // n tokens + 1 token update usage (TODO: +1/+2???)
        else
            delay = delayOffset + (ElementaryDelays.t_r_loop - ElementaryDelays.t_w_loop) * (numTokens + 2); // 1 polling tokens + n tokens + 1 token update usage;


        //if (pCount*t_p+wCount*t_w+rCount*t_r-delayThisLoop > 0)    // n cores (n>2) 
        //    delay = delayOffset + (pCount*t_p+wCount*t_w+rCount*t_r-delayThisLoop)*(numTokens+1); // n tokens + 1 token update usage
        //else
        //    delay = delayOffset + (t_r_loop-t_w_loop)*(numTokens+2); // 1 polling tokens + n tokens + 1 token update usage;
    }
        
    return delay;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

