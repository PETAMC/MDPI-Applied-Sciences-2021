#include <iostream>
#include <setup/sdfapplication.hpp>


SDFApplication::SDFApplication()
    : datahandler(nullptr)
    , codehandler(nullptr)
{
}



SDFApplication::~SDFApplication()
{
    this->CloseApplication();
}



bool SDFApplication::LoadApplication(const char *codepath, const char *datapath)
{
    // Make sure previously opened objects get closed
    this->CloseApplication();

    // Open shared objects
    if(datapath != nullptr)
    {
        this->datahandler = dlopen(datapath, RTLD_LAZY | RTLD_GLOBAL);
        if(this->datahandler == nullptr)
        {
            std::cerr << "\e[1;31mERROR:\e[0m Loading SDF Application failed with error: "
                << dlerror()
                << "\e[0m\n";
            return false;
        }
    }

    this->codehandler = dlopen(codepath, RTLD_LAZY);
    if(this->codehandler == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading SDF Application failed with error: "
            << dlerror()
            << "\e[0m\n";
        return false;
    }

    return true;
}



void* SDFApplication::LoadActor(const char *actorname) noexcept
{
    if(not this->codehandler)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading code for Actor \""
            << actorname
            << "\" failed, because shared object was is not open. "
            << "\e[1;30m(There may be previous errors causing this error)"
            << "\e[0m\n";
        return nullptr;
    }
    void* actor;
   
    actor = dlsym(this->codehandler, actorname);
    if(actor == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading code for Actor \""
            << actorname
            << "\" failed with error: "
            << dlerror()
            << "\n";
        return nullptr;
    }

    return actor;
}



void* SDFApplication::LoadData(const char *symbolname) noexcept
{
    if(not this->datahandler)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading data for \""
            << symbolname
            << "\" failed, because shared object was is not open. "
            << "\e[1;30m(There may be previous errors causing this error)"
            << "\e[0m\n";
        return nullptr;
    }
    void* data;
   
    data = dlsym(this->datahandler, symbolname);
    if(data == nullptr)
    {
        std::cerr << "\e[1;31mERROR:\e[0m Loading data for \""
            << symbolname
            << "\" failed with error: "
            << dlerror()
            << "\n";
        return nullptr;
    }

    return data;
}



void SDFApplication::CloseApplication() noexcept
{
    if(this->codehandler)
        dlclose(this->codehandler);
    if(this->datahandler)
        dlclose(this->datahandler);

    // Ensure defined data
    this->datahandler = nullptr;
    this->codehandler = nullptr;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

