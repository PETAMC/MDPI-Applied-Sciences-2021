#ifndef TILE_HPP
#define TILE_HPP

#include <core/master.hpp>
#include <hardware/memory.hpp>
#include <software/actor.hpp>
#include <monitor.hpp>

class Channel;

class Tile : public core::Master, public PrivateMemory
{
    public:
        Tile(sc_core::sc_module_name name, unsigned int maxiterations = 1000000);
        Tile(sc_core::sc_module_name name, unsigned int maxiterations, Monitor &monitor);
        virtual ~Tile(){};

        Tile& operator<< (Actor& actor); 

        virtual void Execute();
        void WriteWord(sc_dt::uint64 addr, unsigned int* word);
        void ReadWord( sc_dt::uint64 addr, unsigned int* word);

    protected:
        std::vector<Actor*> actors;
        unsigned int        maxiterations;
        std::string         name;

        Monitor             *monitor;   // Can be NULL!
};

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

