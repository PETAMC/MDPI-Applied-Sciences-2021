#ifndef EXPERIMENTSETUP_HPP
#define EXPERIMENTSETUP_HPP

#include <tuple>
#include <unordered_map>
#include <tinyxml2.h>

#include <software/actor.hpp>
#include <software/channel.hpp>
#include <hardware/tile.hpp>
#include <hardware/memory.hpp>
#include <setup/sdfapplication.hpp>

using namespace tinyxml2;

typedef std::unordered_map<std::string, Tile*>  TileMap;
typedef std::unordered_map<std::string, Actor*> ActorMap;
typedef std::unordered_map<std::string, Channel*> ChannelMap;
typedef std::unordered_map<std::string, Memory*> MemoryMap;

class Experiment
{
    public:
        Experiment(std::string &configpath);
        ~Experiment();

        std::tuple<DISTRIBUTION, bool, COMMUNICATIONMODEL> LoadModels();
        bool LoadApplication(SDFApplication *application);
        bool LoadActorMapping(TileMap &tilemap, ActorMap &actormap);
        bool LoadChannelMapping(MemoryMap &memorymap, TileMap &tilemap, ChannelMap &channelmap);

    private:

        XMLDocument xmlfile;
        XMLNode *modelsnode;
        XMLNode *applicationnode;
        XMLNode *actormappingnode;
        XMLNode *channelmappingnode;
};

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

