#ifndef SDFG_SOBEL2_HPP
#define SDFG_SOBEL2_HPP

#include <software/sdf.h>
#include <software/actor.hpp>
#include <monitor.hpp>


namespace Sobel2
{

typedef void    (*GetPixel_t) (token_t position[2], token_t tokensOut[9]);
typedef void    (*GX_t)       (token_t tokensIn[9], token_t tokensOut[1]);
typedef void    (*GY_t)       (token_t tokensIn[9], token_t tokensOut[1]);
typedef token_t (*ABS_t)      (token_t tokensIn1[1], token_t tokensIn2[1]);

// GetPixel /////

class GetPixel: public Actor
{
    public:
        GetPixel(std::string name, DelayVectorMap &delaymap, Monitor &monitor, SDFApplication &application, bool enabledd=false)
            : Actor(name, delaymap, monitor, application) {this->datadependentdelay=enabledd;};

        void Initialize() override;

    protected:
        void ReadPhase() override;
        void ComputePhase() override;
        void WritePhase() override;

    private:
        GetPixel_t GetPixelFunction;
        uint8_t ImageHeight;
        uint8_t ImageWidth;

        token_t position[2];
        token_t tokens[9];

        bool datadependentdelay;
};

void GetPixel::Initialize()
{
    token_t position[2] = {0,0};
    this->channels_out[2]->WriteTokens(position);

    this->GetPixelFunction = (GetPixel_t) this->Actor::application->LoadActor("GetPixel");
    if(this->GetPixelFunction == nullptr)
        throw std::runtime_error("Cannot load GetPixel function");

    uint8_t *byteptr;
    byteptr = (uint8_t*) this->Actor::application->LoadData("ImageHeight");
    if(byteptr == nullptr)
        throw std::runtime_error("Cannot load ImageHeight");
    this->ImageHeight = *byteptr;
    byteptr = (uint8_t*) this->Actor::application->LoadData("ImageWidth");
    if(byteptr == nullptr)
        throw std::runtime_error("Cannot load ImageWidth");
    this->ImageWidth = *byteptr;

    if(this->datadependentdelay)
        std::cerr << "\e[1;33mWARNING:\e[0m Data Dependent Delays only represent MicroBlazes with Extended ALU.\e[0m\n";
}
void GetPixel::ReadPhase()
{
    this->channels_in[0]->ReadTokens(this->position);
}
void GetPixel::ComputePhase()
{
    if(this->datadependentdelay)
    {
        auto x = this->position[0];
        auto y = this->position[1];

        if(x == 0)
        {
            if(y == 0)
                sc_core::wait(sc_core::sc_time(1401, sc_core::SC_NS));  // top left
            else if(y == this->ImageHeight-1)
                sc_core::wait(sc_core::sc_time(1452, sc_core::SC_NS));  // bottom left
            else
                sc_core::wait(sc_core::sc_time(1431, sc_core::SC_NS));  // left edge
        }
        else if(x == this->ImageWidth-1)
        {
            if(y == 0)
                sc_core::wait(sc_core::sc_time(1500, sc_core::SC_NS));  // top right
            else if(y == this->ImageHeight-1)
                sc_core::wait(sc_core::sc_time(1551, sc_core::SC_NS));  // bottom right
            else
                sc_core::wait(sc_core::sc_time(1530, sc_core::SC_NS));  // right edge
        }
        else
        {
            if(y == 0)
                sc_core::wait(sc_core::sc_time(1431, sc_core::SC_NS));  // top edge
            else if(y == this->ImageHeight-1)
                sc_core::wait(sc_core::sc_time(1482, sc_core::SC_NS));  // bottom edge
            else
                sc_core::wait(sc_core::sc_time(1461, sc_core::SC_NS));  // normal case
        }
    }
    else
    {
        sc_core::wait(this->delayvector->GetDelay());
    }
    this->GetPixelFunction(this->position, this->tokens);
}
void GetPixel::WritePhase()
{
    this->channels_out[0]->WriteTokens(this->tokens);
    this->channels_out[1]->WriteTokens(this->tokens);
    this->channels_out[2]->WriteTokens(this->position);
}



// GX /////

class GX: public Actor
{
    public:
        GX(std::string name, DelayVectorMap &delaymap, Monitor &monitor, SDFApplication &application, bool enabledd=false)
            : Actor(name, delaymap, monitor, application) {this->datadependentdelay=enabledd;};

        void Initialize() override;

    protected:
        void ReadPhase() override;
        void ComputePhase() override;
        void WritePhase() override;

    private:
        GX_t GXFunction;
        token_t tokens_in[9];
        token_t tokens_out[1];

        bool datadependentdelay;
};

void GX::Initialize()
{
    this->GXFunction = (GX_t) this->Actor::application->LoadActor("GX");
    if(this->GXFunction == nullptr)
        throw std::runtime_error("Cannot load GX function");

    if(this->datadependentdelay)
        std::cerr << "\e[1;33mWARNING:\e[0m Data Dependent Delays only represent MicroBlazes with Extended ALU.\e[0m\n";
}
void GX::ReadPhase()
{
    this->channels_in[0]->ReadTokens(this->tokens_in);
}
void GX::ComputePhase()
{
    if(this->datadependentdelay)
    {
        sc_core::wait(sc_core::sc_time(478, sc_core::SC_NS));
    }
    else
    {
        sc_core::wait(this->delayvector->GetDelay());
    }

    this->GXFunction(this->tokens_in, this->tokens_out);
}
void GX::WritePhase()
{
    this->channels_out[0]->WriteTokens(this->tokens_out);
}



// GY /////

class GY: public Actor
{
    public:
        GY(std::string name, DelayVectorMap &delaymap, Monitor &monitor, SDFApplication &application, bool enabledd=false)
            : Actor(name, delaymap, monitor, application) {this->datadependentdelay=enabledd;};

        void Initialize() override;

    protected:
        void ReadPhase() override;
        void ComputePhase() override;
        void WritePhase() override;

    private:
        GY_t GYFunction;
        token_t tokens_in[9];
        token_t tokens_out[1];

        bool datadependentdelay;
};

void GY::Initialize()
{
    this->GYFunction = (GY_t) this->Actor::application->LoadActor("GY");
    if(this->GYFunction == nullptr)
        throw std::runtime_error("Cannot load GY function");

    if(this->datadependentdelay)
        std::cerr << "\e[1;33mWARNING:\e[0m Data Dependent Delays only represent MicroBlazes with Extended ALU.\e[0m\n";
}
void GY::ReadPhase()
{
    this->channels_in[0]->ReadTokens(this->tokens_in);
}
void GY::ComputePhase()
{
    if(this->datadependentdelay)
    {
        sc_core::wait(sc_core::sc_time(478, sc_core::SC_NS));
    }
    else
    {
        sc_core::wait(this->delayvector->GetDelay());
    }

    this->GYFunction(this->tokens_in, this->tokens_out);
}
void GY::WritePhase()
{
    this->channels_out[0]->WriteTokens(this->tokens_out);
}



// ABS /////

class ABS: public Actor
{
    public:
        ABS(std::string name, DelayVectorMap &delaymap, Monitor &monitor, SDFApplication &application, bool enabledd=false)
            : Actor(name, delaymap, monitor, application) {this->datadependentdelay=enabledd;};

        void Initialize() override;

    protected:
        void ReadPhase() override;
        void ComputePhase() override;

    private:
        ABS_t ABSFunction;
        token_t tokens_in_x[1];
        token_t tokens_in_y[1];

        bool datadependentdelay;
};

void ABS::Initialize()
{
    this->ABSFunction = (ABS_t) this->Actor::application->LoadActor("ABS");
    if(this->ABSFunction == nullptr)
        throw std::runtime_error("Cannot load ABS function");

    if(this->datadependentdelay)
        std::cerr << "\e[1;33mWARNING:\e[0m Data Dependent Delays only represent MicroBlazes with Extended ALU.\e[0m\n";
}
void ABS::ReadPhase()
{
    this->channels_in[0]->ReadTokens(this->tokens_in_x);
    this->channels_in[1]->ReadTokens(this->tokens_in_y);
}
void ABS::ComputePhase()
{
    if(this->datadependentdelay)
    {
        bool neg1 = this->tokens_in_x[0] < 0;
        bool neg2 = this->tokens_in_y[0] < 0;

        if(neg1 and neg2)
            sc_core::wait(sc_core::sc_time(71, sc_core::SC_NS));
        else if(neg1 or neg2)
            sc_core::wait(sc_core::sc_time(62, sc_core::SC_NS));
        else
            sc_core::wait(sc_core::sc_time(53, sc_core::SC_NS));
    }
    else
    {
        sc_core::wait(this->delayvector->GetDelay());
    }

    token_t result;
    result = this->ABSFunction(this->tokens_in_x, this->tokens_in_y);
    
    const  int image_width  = 48;
    const  int image_height = 48;
    static int iteration;

    auto output = std::to_string(result);
    iteration++;
    if((iteration)%image_width == 0)
        output += "\n";
    else
        output += ", ";
    this->monitor->PrintAppOutput(output);
}


} // namespace

#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

