#ifndef AXI_INTERCONNECT_HPP
#define AXI_INTERCONNECT_HPP

#include <hardware/interconnect.hpp>
#include <atomic>
#include <unistd.h>
#include <functional>
#include <thread>
#include <queue>

class AXIInterconnect
    : public Interconnect
{
    public:
        AXIInterconnect(const char* name);
    
    private:
        virtual void b_transport(int id, tlm::tlm_generic_payload& trans, sc_core::sc_time& delay);
        void PenaltyWait(tlm::tlm_command command);

        void Arbitrate();
        void Release();

        std::atomic<int>  contender;
        sc_core::sc_event busrelease;
        std::queue<size_t> requestqueue;

        std::vector<int>  readpenalties;
        std::vector<int>  writepenalties;
};


#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

