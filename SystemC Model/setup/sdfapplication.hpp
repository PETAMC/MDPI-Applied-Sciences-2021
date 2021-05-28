#ifndef SDFAPPLICATION_HPP
#define SDFAPPLICATION_HPP
#include <dlfcn.h>

class SDFApplication
{
    public:
        SDFApplication();
        ~SDFApplication();

        bool  LoadApplication(const char *codepath, const char *datapath=nullptr); // already open objects will be closed before
        void* LoadActor(const char *actorname ) noexcept;
        void* LoadData (const char *symbolname) noexcept;

    private:
        void CloseApplication() noexcept;

        void* datahandler;
        void* codehandler;
};

#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

