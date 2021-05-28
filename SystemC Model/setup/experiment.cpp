#include <stdexcept>
#include <setup/experiment.hpp>
#include <setup/xmlerrors.hpp>
#include <iostream>


Experiment::Experiment(std::string &configpath)
{
    // Guarantee defined data
    this->applicationnode    = nullptr;
    this->actormappingnode   = nullptr;
    this->channelmappingnode = nullptr;

    // Load configuration file
    XMLError retval;
    retval = this->xmlfile.LoadFile(configpath.c_str());
    if(retval != XML_SUCCESS)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading "
                  << configpath
                  << " failed with error \""
                  << xmlerrors[retval]
                  << "\"!\n";
        throw std::runtime_error("Loading experiment configuration failed!");
    }

    // Load root node
    XMLNode *experimentnode;
    experimentnode = this->xmlfile.RootElement();
    if(experimentnode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading "
                  << configpath
                  << "failed. Root element is not <experiment>!\n";
        throw std::runtime_error("Crucial configuration node <experiment> missing!");
    }

    // <application>
    this->applicationnode = experimentnode->FirstChildElement("application");
    if(this->applicationnode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading "
                  << configpath
                  << "failed. Element <experiment><application> missing!\n";
    }

    // <models>
    this->modelsnode = experimentnode->FirstChildElement("models");
    if(this->modelsnode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading "
                  << configpath
                  << "failed. Element <experiment><models> missing!\n";
    }

    // <mapping>
    XMLNode *mappingnode;
    mappingnode = experimentnode->FirstChildElement("mapping");
    if(mappingnode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading "
                  << configpath
                  << "failed. Element <experiment><mapping> missing!\n";
        throw std::runtime_error("Crucial configuration node <experiment><mapping> missing!");
    }

    this->actormappingnode = mappingnode->FirstChildElement("actors");
    if(this->actormappingnode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading "
                  << configpath
                  << "failed. Element <experiment><mapping><actors> missing!\n";
    }

    this->channelmappingnode = mappingnode->FirstChildElement("channels");
    if(this->channelmappingnode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading "
                  << configpath
                  << "failed. Element <experiment><mapping><channels> missing!\n";
    }
}

Experiment::~Experiment()
{
}



std::tuple<DISTRIBUTION, bool, COMMUNICATIONMODEL> Experiment::LoadModels()
{
    if(this->modelsnode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                  << "Element <experiment><models> not loaded! "
                  << "\e[1;30m(This error occurs due to a previous error)\n";
        throw std::runtime_error("Loading experiment configuration failed!");
    }

    // Load child nodes
    XMLElement *computationnode;
    computationnode = this->modelsnode->FirstChildElement("computation");
    if(computationnode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                  << "Element <experiment><models><computation> not set!\n";
        throw std::runtime_error("Loading experiment configuration failed!");
    }

    XMLElement *communicationnode;
    communicationnode = this->modelsnode->FirstChildElement("communication");
    if(communicationnode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                  << "Element <experiment><models><communication> not set!\n";
        throw std::runtime_error("Loading experiment configuration failed!");
    }

    // Load model names
    const char *computationmodelname;
    computationmodelname = computationnode->GetText();
    if(computationmodelname == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                  << "Element <experiment><models><computation> exists but has no value!\n";
        throw std::runtime_error("Loading experiment configuration failed!");
    }

    const char *functionalattribute = computationnode->Attribute("functional");
    if(functionalattribute == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                  << "<computation> element has no functional attribute!\n";
        throw std::runtime_error("Loading experiment configuration failed!");
    }

    const char *communicationmodelname;
    communicationmodelname = communicationnode->GetText();
    if(communicationmodelname == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                  << "Element <experiment><models><communication> exists but has no value!\n";
        throw std::runtime_error("Loading experiment configuration failed!");
    }

    // Process loaded data
    DISTRIBUTION       computationmodel;
    COMMUNICATIONMODEL communicationmodel;
    bool               functional;

    if(strcmp(computationmodelname, "injected data") == 0)
        computationmodel = DISTRIBUTION::INJECTED;
    else if(strcmp(computationmodelname, "gaussian distribution") == 0)
        computationmodel = DISTRIBUTION::GAUSSIAN;
    else if(strcmp(computationmodelname, "uniform distribution") == 0)
        computationmodel = DISTRIBUTION::UNIFORM;
    else if(strcmp(computationmodelname, "WCET") == 0)
        computationmodel = DISTRIBUTION::WCET;
    else if(strcmp(computationmodelname, "average") == 0)
        computationmodel = DISTRIBUTION::AVERAGE;
    else if(strcmp(computationmodelname, "gaussian KDE") == 0)
        computationmodel = DISTRIBUTION::GAUSSIAN_KDE;
    else
    {
        std::cerr << "\e[1;31mERROR:\e[0m Invalid computation model \""
                  << computationmodelname << "\" "
                  << "in element <experiment><models><computation>!\n";
        throw std::runtime_error("Loading experiment configuration failed!");
    }

    if(strcmp(functionalattribute, "true") == 0)
            functional = true;
    else
            functional = false;

    if(strcmp(communicationmodelname, "cycle accurate") == 0)
        communicationmodel = COMMUNICATIONMODEL::CYCLEACCURATE;
    else if(strcmp(communicationmodelname, "transaction level") == 0)
        communicationmodel = COMMUNICATIONMODEL::SYSTEMCEVENTS;
    else if(strcmp(communicationmodelname, "message level") == 0)
        communicationmodel = COMMUNICATIONMODEL::MESSAGELEVEL;
    else
    {
        std::cerr << "\e[1;31mERROR:\e[0m Invalid communication model \""
                  << communicationmodelname << "\" "
                  << "in element <experiment><models><communication>!\n";
        throw std::runtime_error("Loading experiment configuration failed!");
    }

    //return std::make_tuple(DISTRIBUTION::INJECTED, false, COMMUNICATIONMODEL::CYCLEACCURATE);
    return std::make_tuple(computationmodel, functional, communicationmodel);
}



bool Experiment::LoadApplication(SDFApplication *application)
{
    // Load code and data paths
    if(this->applicationnode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                  << "Element <experiment><application> not loaded! "
                  << "\e[1;30m(This error occurs due to a previous error)"
                  << "\e[0m\n";
        return false;
    }

    XMLElement *codenode;
    codenode = this->applicationnode->FirstChildElement("code");
    if(codenode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                  << "Element <experiment><application><code> not set!\n";
        return false;
    }
    const char *codepath;
    codepath = codenode->GetText();
    if(codepath == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                  << "Element <experiment><application><code> exists but has no value!\n";
        return false;
    }

    XMLElement *datanode;
    const char *datapath = nullptr;
    datanode = this->applicationnode->FirstChildElement("data");
    if(datanode == nullptr)
    {
        // Data is optional
        std::cerr << "\e[1;33mWARNING: \e[0m"
                  << "Optional element <experiment><application><data> not set.\n";
        datapath = nullptr;
    }
    else
    {
        datapath = datanode->GetText();
        if(datapath == nullptr)
        {
            // Data is optional
            std::cerr << "\e[1;33mWARNING: \e[0m"
                      << "Optional element <experiment><application><data> exists but has no value.\n";
        }
    }

    // Load application
    if(application == nullptr)
    {
        std::cerr << "\e[1;33mWARNING:\e[0m"
                  << "Application object pointer is nullptr.\n"
                  << "\e[1;30m(Returning successfully anyway, assuming the developer just wanted to test for a valid experiment configuration)"
                  << "\e[0m\n";
        return true;
    }

    bool success;
    success = application->LoadApplication(codepath, datapath);

    return success;
}



bool Experiment::LoadActorMapping(TileMap &tilemap, ActorMap &actormap)
{
    if(this->actormappingnode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                  << "Element <experiment><mapping><actors> not loaded! "
                  << "\e[1;30m(This error occurs due to a previous error)"
                  << "\e[0m\n";
        return false;
    }

    XMLElement *tilenode;
    tilenode = this->actormappingnode->FirstChildElement("tile");
    if(tilenode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                  << "Element <experiment><mapping><actors> has not a single <tile> child!\n";
        return false;
    }

    // For each tile
    while(tilenode != nullptr)
    {
        const char *tilename    = tilenode->Attribute("name");
        const char *tilefeature = tilenode->Attribute("feature");

        if(tilename == nullptr)
        {
            std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                      << "<tile> element has no name attribute!\n";
            return false;
        }

        // Get Tile from TileMap
        Tile *tile;
        try
        {
            tile = tilemap.at(tilename);
        }
        catch(const std::out_of_range &e) 
        {
            std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed with out-of-range exception \""
                      << e.what()
                      << "\". "
                      << "There exists no tile with the name "
                      << tilename
                      << "!\n";
            return false;
        }

        // For each actor
        XMLElement *actornode;
        actornode = tilenode->FirstChildElement("actor"); // may not exist
        while(actornode != nullptr)
        {
            // Get Actor Name
            std::string actorname;
            actorname = actornode->GetText();

            // Get Actor from ActorMap
            Actor *actor;
            try
            {
                actor = actormap.at(actorname);
            }
            catch(const std::out_of_range &e)
            {
                std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed with out-of-range exception \""
                          << e.what()
                          << "\". "
                          << "There exists no actor with the name \""
                          << actorname
                          << "\"!\n";
                return false;
            }

            // Setup Actor feature
            const char* actorfeature = actornode->Attribute("feature");
            try
            {
                if(actorfeature != nullptr)
                    actor->SelectFeature(actorfeature);
                else if(tilefeature != nullptr)
                    actor->SelectFeature(tilefeature);
                else
                    actor->SelectFeature("none");
            }
            catch (const std::exception& e)
            {
                std::cerr << "\e[1;31mERROR:\e[0m Selecting feature for actor "
                          << actorname
                          << " failed with exception \""
                          << e.what()
                          << "\". "
                          << "!\n";
                return false;
            }


            // Apply Mapping
            *tile << *actor;

            // Next Actor
            actornode = actornode->NextSiblingElement();
        }

        // Next Tile
        tilenode = tilenode->NextSiblingElement();
    }

    return true;
}



bool Experiment::LoadChannelMapping(MemoryMap &memorymap, TileMap &tilemap, ChannelMap &channelmap)
{
    if(this->channelmappingnode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                  << "Element <experiment><mapping><channels> not loaded! "
                  << "\e[1;30m(This error occurs due to a previous error)"
                  << "\e[0m\n";
        return false;
    }

    XMLElement *memorynode;
    memorynode = this->channelmappingnode->FirstChildElement("memory");
    if(memorynode == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                  << "Element <experiment><mapping><channels> has not a single <memory> child!\n";
        return false;
    }

    // For each memory
    while(memorynode != nullptr)
    {
        const char *memoryname = memorynode->Attribute("name");

        if(memoryname == nullptr)
        {
            std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                      << "<memory> element has no name attribute!\n";
            return false;
        }

        // Get Shared Memory from MemoryMap
        // or Private Memory from TileMap
        Memory *memory;
        auto sharedmemory = memorymap.find(memoryname);
        auto privatememory= tilemap.find(memoryname);

        if(sharedmemory != memorymap.end())
        {
            memory = sharedmemory->second;
        }
        else if(privatememory != tilemap.end())
        {
            memory = privatememory->second;
        }
        else
        {
            std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed. "
                      << "There exists no memory with the name "
                      << memoryname
                      << "!\n";
            return false;
        }

        // For each actor
        XMLElement *channelnode;
        channelnode = memorynode->FirstChildElement("channel"); // may not exist
        while(channelnode != nullptr)
        {
            // Get Channel name
            std::string channelname;
            channelname = channelnode->GetText();

            // Get Channel from ChannelMap
            Channel *channel;
            try
            {
                channel = channelmap.at(channelname);
            }
            catch(const std::out_of_range &e)
            {
                std::cerr << "\e[1;31mERROR:\e[0m Loading experiment failed with out-of-range exception \""
                          << e.what()
                          << "\". "
                          << "There exists no channel with the name "
                          << channelname
                          << "!\n";
                return false;
            }

            // Apply Mapping
            *memory << *channel;

            // Next Actor
            channelnode = channelnode->NextSiblingElement();
        }

        // Next Tile
        memorynode = memorynode->NextSiblingElement();
    }

    return true;
}



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

