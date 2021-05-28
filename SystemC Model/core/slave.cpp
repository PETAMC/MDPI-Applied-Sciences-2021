#include <core/slave.hpp>
#include <iostream>

namespace core
{

Slave::Slave(sc_core::sc_module_name name)
    : sc_core::sc_module(name)
    , target_socket("target_socket")
{
    this->target_socket.bind(*this);
}



void Slave::b_transport(tlm::tlm_generic_payload& trans, sc_core::sc_time& delay)
{
    sc_dt::uint64 address     = trans.get_address();
    unsigned int  data_length = trans.get_data_length();
    unsigned int  width       = trans.get_streaming_width();
    unsigned int* data        = reinterpret_cast< unsigned int* >(trans.get_data_ptr());

    if(trans.get_byte_enable_ptr() != NULL)
    {
        std::cerr << "\e[1;31m" << this->name() << ": "  << "\e[1;31mInvalid trans.get_byte_enable_ptr value!" << std::endl;
        trans.set_response_status(tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE);
        return;
    }
    else if(width != 4)
    {
        std::cerr << "\e[1;31m" << this->name() << ": "  << "\e[1;31mInvalid streaming_width! Must always be 4" << std::endl;
        trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
        return;
    }
    else if(data_length != 4)
    {
        std::cerr << "\e[1;31m" << this->name() << ": "  << "\e[1;31mInvalid data_length! Must always be 4" << std::endl;
        trans.set_response_status(tlm::TLM_BURST_ERROR_RESPONSE);
        return;
    }

    switch(trans.get_command())
    {
        case tlm::TLM_WRITE_COMMAND:
            this->Write(address, *data);
            break;

        case tlm::TLM_READ_COMMAND:
            *data = this->Read(address);
            break;

        case tlm::TLM_IGNORE_COMMAND:
        default:
            std::cerr << "\e[1;31m" << this->name() << ": "  << "\e[1;31mInvalid command!" << std::endl;
            break;
    }

    trans.set_response_status(tlm::TLM_OK_RESPONSE);
}


tlm::tlm_sync_enum Slave::nb_transport_fw(tlm::tlm_generic_payload&, tlm::tlm_phase&, sc_core::sc_time&)
{
    std::cerr << "\e[0;33mnb_transport_fw called" << std::endl;
    return tlm::TLM_COMPLETED; 
}

bool Slave::get_direct_mem_ptr(tlm::tlm_generic_payload&, tlm::tlm_dmi&)
{
    std::cerr << "\e[0;33mget_direct_mem_ptr called" << std::endl;
    return false; 
}

unsigned int Slave::transport_dbg(tlm::tlm_generic_payload&)
{
    std::cerr << "\e[0;33mtransport_dbg called" << std::endl;
    return 0; 
}

} // namespace core
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

