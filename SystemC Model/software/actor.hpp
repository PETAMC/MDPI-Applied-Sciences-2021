#ifndef ACTOR_HPP
#define ACTOR_HPP

#include <string>
#include <iostream>

#include <core/master.hpp>
#include <delayvector.hpp>
#include <software/channel.hpp>
#include <monitor.hpp>
#include <setup/sdfapplication.hpp>

class Actor
{
    public:
        Actor(std::string name, DelayVectorMap &delaymap, Monitor &monitor, SDFApplication &application);

        void DefineAsStartActor();
        void DefineAsFinishActor();

        void ChangeTile(Tile *tile);
        void SelectFeature(std::string feature);
    
        Actor& operator<< (Channel &incoming); 
        Actor& operator>> (Channel &outgoing); 

        virtual void Initialize();
        void Execute();

    protected:

        virtual void ReadPhase();
        virtual void ComputePhase() = 0;
        virtual void WritePhase();

        std::string name;
        DelayVector *delayvector;
        SDFApplication *application;
        std::vector<Channel*> channels_out;
        std::vector<Channel*> channels_in;
        Monitor *monitor;

    private:
        void TracePhase(const char* phase);

        DelayVectorMap *delayvectormap;
        Tile    *tile;      // The tile this actor gets executed on
        bool    isstartactor;
        bool    isfinishactor;
};

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

