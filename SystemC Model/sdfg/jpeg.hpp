#ifndef SDFG_JPEG_HPP
#define SDFG_JPEG_HPP

#include <software/sdf.h>
#include <software/actor.hpp>
#include <monitor.hpp>

namespace JPEG
{

typedef void (*GetEncodedImageBlock_t)
    (token_t DCOffset[3], token_t Y[8*8], token_t Cr[8*8], token_t Cb[8*8], token_t BlockInfo[4]);
typedef void (*InverseQuantization_t)
    (const uint8_t quantizationtable[8*8], token_t QuantizedCoefficients[8*8], token_t DCTCoefficients[8*8]);
typedef void (*FastIDCT_t)
    (token_t source[8*8], token_t destination[8*8]);
typedef void (*CreateRGBPixels_t)
    (token_t Y[8*8], token_t Cr[8*8], token_t Cb[8*8], token_t Pixels[8*8]);

//typedef uint8_t Quantizationtable_t[8*8];
typedef uint8_t* Quantizationtable_t;

enum component_t {COMP_Y=0, COMP_Cb=1, COMP_Cr=2};
enum blockinfoindex_t {BII_IMG_WIDTH=0, BII_IMG_HEIGHT=1, BII_CURRENTBLOCK=2, BII_MAXBLOCKS=3};

// Get Encoded Image Block/////

class GetEncodedImageBlock: public Actor
{
    public:
        GetEncodedImageBlock(std::string name, DelayVectorMap &delaymap, Monitor &monitor, SDFApplication &application)
            : Actor(name, delaymap, monitor, application) {};

        void Initialize() override;

    protected:
        void ReadPhase() override;
        void ComputePhase() override;
        void WritePhase() override;

    private:
        GetEncodedImageBlock_t Function;
        token_t dcoffset[3];
        token_t Y[64];
        token_t Cr[64];
        token_t Cb[64];
};
void GetEncodedImageBlock::Initialize()
{
    token_t dcoffset[3] = {0,0,0};
    this->channels_out[3]->WriteTokens(dcoffset);

    this->Function = (GetEncodedImageBlock_t) this->Actor::application->LoadActor("GetEncodedImageBlock");
    if(this->Function == nullptr)
        throw std::runtime_error("Cannot load GetEncodedImageBlock function");
}
void GetEncodedImageBlock::ReadPhase()
{
    this->channels_in[0]->ReadTokens(this->dcoffset);
}
void GetEncodedImageBlock::ComputePhase()
{
    this->Function(this->dcoffset, this->Y, this->Cr, this->Cb, NULL);
    sc_core::wait(this->delayvector->GetDelay());
}
void GetEncodedImageBlock::WritePhase()
{
    this->channels_out[0]->WriteTokens(this->Y);
    this->channels_out[1]->WriteTokens(this->Cr);
    this->channels_out[2]->WriteTokens(this->Cb);
    this->channels_out[3]->WriteTokens(this->dcoffset);
}



// IQ /////

class IQ_Y: public Actor
{
    public:
        IQ_Y(std::string name, DelayVectorMap &delaymap, Monitor &monitor, SDFApplication &application)
            : Actor(name, delaymap, monitor, application) {};

        void Initialize() override;

    protected:
        void ReadPhase() override;
        void ComputePhase() override;
        void WritePhase() override;

    private:
        InverseQuantization_t Function;
        Quantizationtable_t   quantizationtable;
        token_t qy[64];
        token_t dqy[64];
};
void IQ_Y::Initialize()
{
    this->Function = (InverseQuantization_t) this->Actor::application->LoadActor("InverseQuantization");
    if(this->Function == nullptr)
        throw std::runtime_error("Cannot load InverseQuantization function");
    this->quantizationtable = (Quantizationtable_t) this->Actor::application->LoadData("Y_quantizationtable");
    if(this->quantizationtable == nullptr)
        throw std::runtime_error("Cannot load Y_quantizationtable");
}
void IQ_Y::ReadPhase()
{
    this->channels_in[0]->ReadTokens(this->qy);
}
void IQ_Y::ComputePhase()
{
    this->Function(this->quantizationtable, this->qy, this->dqy);
    sc_core::wait(this->delayvector->GetDelay());
}
void IQ_Y::WritePhase()
{
    this->channels_out[0]->WriteTokens(this->dqy);
}

class IQ_Cr: public Actor
{
    public:
        IQ_Cr(std::string name, DelayVectorMap &delaymap, Monitor &monitor, SDFApplication &application)
            : Actor(name, delaymap, monitor, application) {};

        void Initialize() override;

    protected:
        void ReadPhase() override;
        void ComputePhase() override;
        void WritePhase() override;

    private:
        InverseQuantization_t Function;
        Quantizationtable_t   quantizationtable;
        token_t qcr[64];
        token_t dqcr[64];
};
void IQ_Cr::Initialize()
{
    this->Function = (InverseQuantization_t) this->Actor::application->LoadActor("InverseQuantization");
    if(this->Function == nullptr)
        throw std::runtime_error("Cannot load InverseQuantization function");
    this->quantizationtable = (Quantizationtable_t) this->Actor::application->LoadData("Cr_quantizationtable");
    if(this->quantizationtable == nullptr)
        throw std::runtime_error("Cannot load Cr_quantizationtable");
}
void IQ_Cr::ReadPhase()
{
    this->channels_in[0]->ReadTokens(this->qcr);
}
void IQ_Cr::ComputePhase()
{
    this->Function(this->quantizationtable, this->qcr, this->dqcr);
    sc_core::wait(this->delayvector->GetDelay());
}
void IQ_Cr::WritePhase()
{
    this->channels_out[0]->WriteTokens(this->dqcr);
}

class IQ_Cb: public Actor
{
    public:
        IQ_Cb(std::string name, DelayVectorMap &delaymap, Monitor &monitor, SDFApplication &application)
            : Actor(name, delaymap, monitor, application) {};

        void Initialize() override;

    protected:
        void ReadPhase() override;
        void ComputePhase() override;
        void WritePhase() override;

    private:
        InverseQuantization_t Function;
        Quantizationtable_t   quantizationtable;
        token_t qcb[64];
        token_t dqcb[64];
};
void IQ_Cb::Initialize()
{
    this->Function = (InverseQuantization_t) this->Actor::application->LoadActor("InverseQuantization");
    if(this->Function == nullptr)
        throw std::runtime_error("Cannot load InverseQuantization function");
    this->quantizationtable = (Quantizationtable_t) this->Actor::application->LoadData("Cb_quantizationtable");
    if(this->quantizationtable == nullptr)
        throw std::runtime_error("Cannot load Cb_quantizationtable");
}
void IQ_Cb::ReadPhase()
{
    this->channels_in[0]->ReadTokens(this->qcb);
}
void IQ_Cb::ComputePhase()
{
    this->Function(this->quantizationtable, this->qcb, this->dqcb);
    sc_core::wait(this->delayvector->GetDelay());
}
void IQ_Cb::WritePhase()
{
    this->channels_out[0]->WriteTokens(this->dqcb);
}



// IDCT /////

class IDCT_Y: public Actor
{
    public:
        IDCT_Y(std::string name, DelayVectorMap &delaymap, Monitor &monitor, SDFApplication &application)
            : Actor(name, delaymap, monitor, application) {};

        void Initialize() override;

    protected:
        void ReadPhase() override;
        void ComputePhase() override;
        void WritePhase() override;

    private:
        FastIDCT_t Function;
        token_t enc[64];
        token_t dec[64];
};
void IDCT_Y::Initialize()
{
    this->Function = (FastIDCT_t) this->Actor::application->LoadActor("FastIDCT");
    if(this->Function == nullptr)
        throw std::runtime_error("Cannot load FastIDCT function");
}
void IDCT_Y::ReadPhase()
{
    this->channels_in[0]->ReadTokens(this->enc);
}
void IDCT_Y::ComputePhase()
{
    this->Function(this->enc, this->dec);
    sc_core::wait(this->delayvector->GetDelay());
}
void IDCT_Y::WritePhase()
{
    this->channels_out[0]->WriteTokens(this->dec);
}

class IDCT_Cr: public Actor
{
    public:
        IDCT_Cr(std::string name, DelayVectorMap &delaymap, Monitor &monitor, SDFApplication &application)
            : Actor(name, delaymap, monitor, application) {};

        void Initialize() override;

    protected:
        void ReadPhase() override;
        void ComputePhase() override;
        void WritePhase() override;

    private:
        FastIDCT_t Function;
        token_t enc[64];
        token_t dec[64];
};
void IDCT_Cr::Initialize()
{
    this->Function = (FastIDCT_t) this->Actor::application->LoadActor("FastIDCT");
    if(this->Function == nullptr)
        throw std::runtime_error("Cannot load FastIDCT function");
}
void IDCT_Cr::ReadPhase()
{
    this->channels_in[0]->ReadTokens(this->enc);
}
void IDCT_Cr::ComputePhase()
{
    this->Function(this->enc, this->dec);
    sc_core::wait(this->delayvector->GetDelay());
}
void IDCT_Cr::WritePhase()
{
    this->channels_out[0]->WriteTokens(this->dec);
}

class IDCT_Cb: public Actor
{
    public:
        IDCT_Cb(std::string name, DelayVectorMap &delaymap, Monitor &monitor, SDFApplication &application)
            : Actor(name, delaymap, monitor, application) {};

        void Initialize() override;

    protected:
        void ReadPhase() override;
        void ComputePhase() override;
        void WritePhase() override;

    private:
        FastIDCT_t Function;
        token_t enc[64];
        token_t dec[64];
};
void IDCT_Cb::Initialize()
{
    this->Function = (FastIDCT_t) this->Actor::application->LoadActor("FastIDCT");
    if(this->Function == nullptr)
        throw std::runtime_error("Cannot load FastIDCT function");
}
void IDCT_Cb::ReadPhase()
{
    this->channels_in[0]->ReadTokens(this->enc);
}
void IDCT_Cb::ComputePhase()
{
    this->Function(this->enc, this->dec);
    sc_core::wait(this->delayvector->GetDelay());
}
void IDCT_Cb::WritePhase()
{
    this->channels_out[0]->WriteTokens(this->dec);
}



// Create RGB Pixels /////

class CreateRGBPixels: public Actor
{
    public:
        CreateRGBPixels(std::string name, DelayVectorMap &delaymap, Monitor &monitor, SDFApplication &application)
            : Actor(name, delaymap, monitor, application) {};

        void Initialize() override;

    protected:
        void ReadPhase() override;
        void ComputePhase() override;

    private:
        void PrintImageChunk(token_t pixels[64]);
        
        CreateRGBPixels_t Function;
        uint16_t Image_xdimension;
        uint16_t Image_ydimension;

        token_t y[64];
        token_t cr[64];
        token_t cb[64];
};
void CreateRGBPixels::Initialize()
{
    this->Function = (CreateRGBPixels_t) this->Actor::application->LoadActor("CreateRGBPixels");
    if(this->Function == nullptr)
        throw std::runtime_error("Cannot load CreateRGBPixels function");

    uint16_t *wordptr;
    wordptr = (uint16_t*) this->Actor::application->LoadData("Image_xdimension");
    if(wordptr == nullptr)
        throw std::runtime_error("Cannot load Image_xdimension");
    this->Image_xdimension = *wordptr;

    wordptr = (uint16_t*) this->Actor::application->LoadData("Image_ydimension");
    if(wordptr == nullptr)
        throw std::runtime_error("Cannot load Image_ydimension");
    this->Image_ydimension = *wordptr;
}
void CreateRGBPixels::ReadPhase()
{
    this->channels_in[0]->ReadTokens(this->y);
    this->channels_in[1]->ReadTokens(this->cr);
    this->channels_in[2]->ReadTokens(this->cb);
}
void CreateRGBPixels::ComputePhase()
{
    token_t pixels[64];
    this->Function(this->y, this->cr, this->cb, pixels);
    this->PrintImageChunk(pixels);
    sc_core::wait(this->delayvector->GetDelay());
}
void CreateRGBPixels::PrintImageChunk(token_t pixels[64])
{
    std::string output = "";

    static int block_x, block_y;
    if(block_x == 0 && block_y == 0)
        output += "\e[1;1H\e[0m\e[2J";   // Begin of screen and clear

    for(int y=0; y<8; y++)
    {
        for(int x=0; x<8; x++)
        {

            auto pixel = static_cast<int>(pixels[y*8+x]);
            auto r     = std::to_string((pixel >>  0) & 0x000000FF);
            auto g     = std::to_string((pixel >>  8) & 0x000000FF);
            auto b     = std::to_string((pixel >> 16) & 0x000000FF);

            output += "\e[48;2;";
            output += r;
            output += ";";
            output += g;
            output += ";";
            output += b;
            output += "m  "; // 2 spaces -> 1 pixel
        }
        output += "\e[16D\e[1B"; // 8 left, 1 down
    }

    block_x++;
    if(block_x >= this->Image_xdimension/8)
    {
        block_x = 0;
        block_y++;
        output += "\e[1G"; // Begin of line
        if(block_y >= this->Image_ydimension/8)
            block_y = 0;
    }
    else
    {
        output += "\e[8A\e[16C"; // 8 up, 8 right
    }

    this->monitor->PrintAppOutput(output);
}


} // namespace

#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

