#include <plot.hpp>
#include <iostream>
#include <sstream>
#include <cmath>
#include <algorithm>


Plot::Plot(const std::string &path, double width, double height)
    : VectorGraphic(path, width, height)
    , drawmaximum(false)
    , drawaverage(false)
{

    // Positions [cm]
    const double axiswidth  = 0.2;
    const double textheight = 0.3;
    this->histogramposition.x = 1.4;
    this->histogramposition.y = 0.1 + textheight/2; // some overshoot of the y axis
    this->histogramsize.w     = this->pagesize.w - this->histogramposition.x - 0.7;
    this->histogramsize.h     = this->pagesize.h - this->histogramposition.y - 1.3;

    // Axis
    this->xaxissize.w     = this->histogramsize.w;
    this->xaxissize.h     = axiswidth;
    this->xaxisposition.x = this->histogramposition.x;
    this->xaxisposition.y = this->histogramposition.y + this->histogramsize.h;

    this->yaxissize.w     = axiswidth;
    this->yaxissize.h     = this->histogramsize.h;
    this->yaxisposition.x = this->histogramposition.x - this->yaxissize.w;
    this->yaxisposition.y = this->histogramposition.y;

    // Axis Labels
    const double xlabelheight = textheight; // \_ Text width and height
    const double ylabelwidth  = 0.6;        // /
    const double axislabelgap = 0.1;        // Gap between the axis ticks and their labels

    this->xlabelssize.w     = this->xaxissize.w;
    this->xlabelssize.h     = xlabelheight;
    this->xlabelsposition.x = this->xaxisposition.x;
    this->xlabelsposition.y = this->xaxisposition.y + this->xaxissize.h + axislabelgap;

    this->ylabelssize.w     = ylabelwidth;
    this->ylabelssize.h     = this->yaxissize.h;
    this->ylabelsposition.x = this->yaxisposition.x - this->ylabelssize.w - axislabelgap;
    this->ylabelsposition.y = this->yaxisposition.y;

    // Axis Legends
    const double axislegendgap = 0.2;       // Gap between the axis label and the legends
    this->xlegendsize.w     = this->xaxissize.w;
    this->xlegendsize.h     = textheight;
    this->xlegendposition.x = this->xaxisposition.x;
    this->xlegendposition.y = this->xlabelsposition.y + this->xlabelssize.h + axislegendgap;

    this->ylegendsize.w     = textheight;
    this->ylegendsize.h     = this->yaxissize.h;
    this->ylegendposition.x = this->ylabelsposition.x - this->ylegendsize.w - axislegendgap;
    this->ylegendposition.y = this->yaxisposition.y;

    // Colors [r,g,b]
    this->axiscolor             = {.r=0.3, .g=0.3, .b=0.3, .a=1.0};
    this->axislabelcolor        = {.r=0.1, .g=0.1, .b=0.1, .a=1.0};
    this->axislegendcolor       = {.r=0.1, .g=0.1, .b=0.1, .a=1.0};
    this->gridcolor             = {.r=0.8, .g=0.8, .b=0.8, .a=1.0};
    this->legendlinecolor       = {.r=0.8, .g=0.8, .b=0.8, .a=1.0};
    this->legendbackgroundcolor = {.r=1.0, .g=1.0, .b=1.0, .a=0.5};

    // Widths [pt]
    this->axislinewidth   = 1.5;
    this->gridlinewidth   = 0.5;
    this->markerlinewidth = 1.5;
    this->legendlinewidth = 1.5;

    // Offsets [cm]
    this->legendoffset  = 0.1;

    // Ticks []
    this->xaxisticks = static_cast<int>(pagesize.w / 1.6); // 1.6: works good for 13.5cm and 7.0cm
    this->yaxisticks = static_cast<int>(pagesize.h);

    // Other
    this->legendvalign = VAlign::Left;
}


Plot::~Plot()
{
}



void Plot::SetLabels(const std::string &xaxis, const std::string &yaxis)
{
    this->xaxislegend = xaxis;
    this->yaxislegend = yaxis;
    return;
}



void Plot::SetMaximumMarker(bool enable)
{
    this->drawmaximum = enable;
}
void Plot::SetAverageMarker(bool enable)
{
    this->drawaverage = enable;
}
void Plot::SetLegendVAlignment(const std::string &alignment)
{
    // Just make the strings content all lower case
    // I ♥ C++
    std::string valign(alignment);
    try
    {
        std::transform(
                valign.begin(),  // First input
                valign.end(),    // Last input
                valign.begin(),  // First output
                [](unsigned char c){ return std::tolower(c); }
            );
    }
    catch(std::exception &e)
    {
        std::cerr << "\e[1;31mERROR: " << e.what()
                  << ". Failed making \"" << alignment << "\" all lower case.\n";
        return;
    }

    // Check for alignment
    if(valign == "left")
        this->SetLegendVAlignment(VAlign::Left);
    else if(valign == "center")
        this->SetLegendVAlignment(VAlign::Center);
    else if(valign == "right")
        this->SetLegendVAlignment(VAlign::Right);
    else
        std::cerr << "\e[1;31mERROR: "
                  << "Alignment \"" << valign << "\" unknown. Only \"left\", \"center\" and \"right\" allowed.\n";
    return;
}
void Plot::SetLegendVAlignment(VAlign valign)
{
    this->legendvalign = valign;
}



void Plot::Draw(const std::vector<Histogram> &histograms)
{
#if 0
    this->DebugFrame(this->xaxisposition,     this->xaxissize);
    this->DebugFrame(this->yaxisposition,     this->yaxissize);
    this->DebugFrame(this->xlabelsposition,   this->xlabelssize);
    this->DebugFrame(this->ylabelsposition,   this->ylabelssize);
    this->DebugFrame(this->xlegendposition,   this->xlegendsize);
    this->DebugFrame(this->ylegendposition,   this->ylegendsize);
    this->DebugFrame(this->histogramposition, this->histogramsize);
#endif

    // Determine axis settings
    this->xmin = std::numeric_limits<long>::max();
    this->xmax = std::numeric_limits<long>::min();
    this->ymin = 0.0;
    this->ymax = std::numeric_limits<double>::min();

    double ymax2nd = std::numeric_limits<double>::min(); // Second highest value.
    // If the second highest y value is only have has much as the highest value
    //  ymax >= 2 · ymax2nd
    // then ymax2nd will be used as highest value for the y-axis.
    // The original ymax will be discard. The rendering cuts of the highest bin (ymax).

    for(auto histogram : histograms)
    {
        this->legend.emplace_back(histogram.GetLabel(), this->VectorGraphic::CreateColor(histogram.GetColor()));

        const BinDefinition &bin = histogram.GetBinDefinition();
        if(this->xmin > bin.from)
            this->xmin = bin.from;
        if(this->xmax < bin.to)
            this->xmax = bin.to;

        for(auto rawdata : histogram.GetRawData())
        {
            if(this->ymax <= rawdata.second)
                this->ymax = rawdata.second;
            else if(ymax2nd < rawdata.second)
                ymax2nd = rawdata.second;
        }
    }
    //std::cout << "\e[1;37mxmin: " << xmin << "\e[0m\n";
    //std::cout << "\e[1;37mxmax: " << xmax << "\e[0m\n";
    //std::cout << "\e[1;37mymin: " << ymin << "\e[0m\n";
    //std::cout << "\e[1;37mymax: " << ymax << "\e[0m\n";
    //std::cout << "\e[1;37mymax2nd: " << ymax2nd << "\e[0m\n";
    if(ymax2nd * 2 < this->ymax)
    {
        this->ymax = ymax2nd;
    }

    // Optimizing Y-Axis settings
    {
    double delta = (this->ymax - this->ymin) / (this->yaxisticks - 1); // -1: ticks->segments
    //std::cout << "delta: " << delta << "\n";
    long idelta = static_cast<long>(delta * 100.0) + 1;   // +1 Round to next higher integer
    //std::cout << "tmp:   " << idelta << "\n";
    delta = static_cast<double>(idelta) / 100.0;
    //std::cout << "delta: " << delta << "\n";
    this->ymax = delta * (this->yaxisticks - 1);
    }
    
    // Optimizing X-Axis settings
    {
    long delta  = (this->xmax - this->xmin) / (this->xaxisticks - 1);
    //long digits = std::log10(this->xmin) + 1;
    long digits = std::log10(delta) + 1;
    long factor = std::pow(10, digits/2);
    //std::cout << "factor: " << factor << "\n";
    delta     += factor - delta % factor;   // Fill to next factor-level
    //std::cout << "delta:  " << delta << "\n";
    this->xmin = this->xmin - this->xmin % factor;
    this->xmax = this->xmin + delta * (this->xaxisticks - 1);
    }

    // Calculate values depending on Axis
    this->xscale = this->xaxissize.w / (this->xmax - this->xmin);
    this->yscale = this->yaxissize.h / (this->ymax - this->ymin);

    //std::cout << "\e[1;37mopt xmin: " << xmin << "\e[0m\n";
    //std::cout << "\e[1;37mopt xmax: " << xmax << "\e[0m\n";
    //std::cout << "\e[1;37mopt ymin: " << ymin << "\e[0m\n";
    //std::cout << "\e[1;37mopt ymax: " << ymax << "\e[0m\n";


    // Draw Data
    for(auto histogram : histograms)
    {
        this->DrawHistogram(histogram);
    }

    this->DrawGrid();   // Grid between histogram and markers

    for(auto histogram : histograms)
    {
        if(this->drawmaximum)
            this->DrawMaximum(histogram);
        if(this->drawaverage)
            this->DrawAverage(histogram);
    }

    // Draw rest of the plot
    this->DrawXAxis();
    this->DrawYAxis();
    this->DrawXLabels();
    this->DrawYLabels();
    this->DrawXLegend();
    this->DrawYLegend();
    this->DrawLegend();
    return;
}


void Plot::DebugFrame(Position &pos, Size &size)
{
    Color color = {.r=1.0, .g=0.0, .b=0.0, .a=1.0};
    Color bg    = {.r=1.0, .g=1.0, .b=1.0, .a=1.0};
    this->VectorGraphic::Rectangle(pos, size, color, bg);
    return;
}


void Plot::DrawGrid()
{
    Position from;
    Position to;
    double   distance;

    // Horizontal
    from     = this->histogramposition;
    to.x     = from.x + this->histogramsize.w;
    distance = this->yaxissize.h / (this->yaxisticks - 1); // x ticks = x-1 segments

    for(int i=0; i<this->yaxisticks; i++)
    {
        from.y = this->yaxisposition.y + i * distance;
        to.y   = from.y;
        this->VectorGraphic::Line(from, to, this->gridcolor, this->gridlinewidth);
    }

    // Vertical
    from     = this->histogramposition;
    to.y     = from.y + this->histogramsize.h;
    distance = this->xaxissize.w / (this->xaxisticks - 1); // x ticks = x-1 segments

    for(int i=0; i<this->xaxisticks; i++)
    {
        from.x = this->xaxisposition.x + i * distance;
        to.x   = from.x;
        this->VectorGraphic::Line(from, to, this->gridcolor, this->gridlinewidth);
    }

    return;
}



void Plot::DrawXAxis()
{
    Position from;
    Position to;

    // Draw Axis-Line
    from = this->xaxisposition;
    to.x = this->xaxisposition.x + this->xaxissize.w;
    to.y = this->xaxisposition.y;
    this->VectorGraphic::Line(from, to, this->axiscolor, this->axislinewidth);

    // Draw Ticks
    to.x = this->xaxisposition.x;
    to.y = this->xaxisposition.y + this->xaxissize.h;

    double tickdistance = this->xaxissize.w / (this->xaxisticks - 1); // x ticks = x-1 segments
    for(int i=0; i<this->xaxisticks; i++)
    {
        from.x = this->xaxisposition.x + i * tickdistance;
        to.x   = from.x;
        this->VectorGraphic::Line(from, to, this->axiscolor, this->axislinewidth);
    }

    return;
}



void Plot::DrawYAxis()
{
    Position from;
    Position to;

    // Draw Axis-Line
    from.x = this->yaxisposition.x + this->yaxissize.w;
    from.y = this->yaxisposition.y;
    to.x   = this->yaxisposition.x + this->yaxissize.w;
    to.y   = this->yaxisposition.y + this->yaxissize.h;
    this->VectorGraphic::Line(from, to, this->axiscolor, this->axislinewidth);

    // Draw Ticks
    from = this->yaxisposition;
    to.x = this->yaxisposition.x + this->yaxissize.w;
    to.y = this->yaxisposition.y;

    double tickdistance = this->yaxissize.h / (this->yaxisticks - 1); // x ticks = x-1 segments
    for(int i=0; i<this->yaxisticks; i++)
    {
        from.y = this->yaxisposition.y + i * tickdistance;
        to.y   = from.y;
        this->VectorGraphic::Line(from, to, this->axiscolor, this->axislinewidth);
    }

    return;
}



void Plot::DrawXLabels()
{
    long   value        = this->xmin; // start at bottom
    long   delta        = (this->xmax - this->xmin) / (this->xaxisticks - 1); // increase x per segment
    double tickdistance = this->xlabelssize.w / (this->xaxisticks - 1); // x ticks = x-1 segments

    Position pos;
    pos.x = this->xlabelsposition.x;  // Starting from left
    pos.y = this->xlabelsposition.y;  // For top-center aligned text

    for(int i=this->xaxisticks; i>0; i--)
    {
        std::string label = std::to_string(value);
        this->VectorGraphic::Text(pos, this->axislabelcolor, label, HAlign::Top, VAlign::Center);
        pos.x += tickdistance;
        value += delta;
    }
    return;
}



void Plot::DrawYLabels()
{
    double value        = this->ymin; // start at bottom
    double delta        = (this->ymax - this->ymin) / (this->yaxisticks - 1); // increase x per segment
    double tickdistance = this->ylabelssize.h / (this->yaxisticks - 1); // x ticks = x-1 segments

    Position pos;
    pos.x = this->ylabelsposition.x + this->ylabelssize.w;  // For right aligned text
    pos.y = this->ylabelsposition.y + this->ylabelssize.h;  // Starting from bottom

    for(int i=this->yaxisticks; i>0; i--)
    {
        std::ostringstream converter;
        converter.precision(2);
        converter << std::fixed << value;

        std::string label = converter.str();
        this->VectorGraphic::Text(pos, this->axislabelcolor, label, HAlign::Center, VAlign::Right);

        pos.y -= tickdistance;
        value += delta;
    }
    return;
}



void Plot::DrawXLegend()
{
    Position pos;
    pos.x = this->xlegendposition.x + this->xlegendsize.w / 2;
    pos.y = this->xlegendposition.y;
    this->VectorGraphic::Text(pos, this->axislegendcolor, this->xaxislegend, HAlign::Top, VAlign::Center);
}



void Plot::DrawYLegend()
{
    Position pos;
    pos.x = this->ylegendposition.x;
    pos.y = this->ylegendposition.y + this->ylegendsize.h / 2;
    this->VectorGraphic::Text(pos, this->axislegendcolor, this->yaxislegend, HAlign::Center, VAlign::Left, true);
}



void Plot::DrawHistogram(const Histogram &histogram)
{
    // Load Histogram Configuration
    const BinDefinition &bin       = histogram.GetBinDefinition();
    const std::string   &colorcode = histogram.GetColor();
    Color                color     = this->VectorGraphic::CreateColor(colorcode);
    color.a = 0.5;  // Make bins transparent

    // Draw Histogram
    Position offset;
    offset.x = this->histogramposition.x;                           // \_ Bottom left corner
    offset.y = this->histogramposition.y + this->histogramsize.h;   // /
    for(auto rawdata : histogram.GetRawData())
    {
        Position pos;
        Size     size;

        // Calculate positions
        double value;
        value  = bin.from + rawdata.first * bin.size;
        value -= this->xmin;    // remove offset (start of X-Axis)

        double probability;
        probability = rawdata.second;
        // If the probability exceeds the y-axis limit, draw a cut-off bin
        if(probability > this->ymax)
        {
            pos.x  = offset.x + value * this->xscale;
            pos.y  = offset.y - this->ymax * this->yscale;
            size.w = bin.size * xscale;
            size.h = this->ymax * yscale;

            // Draw Arrow
            Position arrowstart = {.x = pos.x+size.w/2, .y = pos.y};
            Position arrowend   = {.x = arrowstart.x,   .y = arrowstart.y*PT2CM(10)}; // 10pt: Font Size
            Color    arrowcolor = {.r = color.r, .g = color.g, .b = color.b, .a = 1.0};
            this->VectorGraphic::Arrow(arrowstart, arrowend, arrowcolor, 1.5/*pt*/);

            // Draw Label
            std::ostringstream converter;
            converter.precision(2);
            converter << std::fixed << probability;

            std::string label      = converter.str();
            Position    labelpos   = {.x = pos.x+size.w, .y = pos.y};
            Color       labelcolor = {.r = color.r, .g = color.g, .b = color.b, .a = 1.0};
            this->VectorGraphic::Text(labelpos, labelcolor, label, HAlign::Center, VAlign::Left);
        }
        else
        {
            pos.x  = offset.x + value * this->xscale;
            pos.y  = offset.y - probability * this->yscale;
            size.w = bin.size * xscale;
            size.h = probability * yscale;
        }

        // Draw bin
        this->VectorGraphic::Rectangle(pos, size, color, color, 0.0);
    }

    return;
}



void Plot::DrawMaximum(const Histogram &histogram)
{
    // Get Data From Histogram
    long                 maximum   = histogram.GetMaximum();
    const std::string   &colorcode = histogram.GetColor();

    // Prepare Line
    Position from;
    Position to;
    Color    color = this->VectorGraphic::CreateColor(colorcode);

    // Draw Line
    from.x = this->histogramposition.x + (maximum - this->xmin) * this->xscale;
    from.y = this->histogramposition.y;
    to.x   = from.x;
    to.y   = from.y + this->histogramsize.h;
    this->VectorGraphic::Line(from, to, color, this->markerlinewidth);

    return;
}



void Plot::DrawAverage(const Histogram &histogram)
{
    // Get Data From Histogram
    long                 average   = histogram.GetAverage();
    const std::string   &colorcode = histogram.GetColor();

    // Prepare Line
    Position from;
    Position to;
    Color    color = this->VectorGraphic::CreateColor(colorcode);

    // Draw Line
    from.x = this->histogramposition.x + (average - this->xmin) * this->xscale;
    from.y = this->histogramposition.y;
    to.x   = from.x;
    to.y   = from.y + this->histogramsize.h;

    static double dashoffset = 0.0;
    const  double dashlength = 5.0;
    this->VectorGraphic::DashedLine(from, to, color, dashlength, this->markerlinewidth, dashoffset);
    dashoffset += 2.5;

    return;
}



void Plot::DrawLegend()
{
    double       maxtextheight = 0.0;
    double       maxtextwidth  = 0.0;
    const double textmargin    = 0.2; // Space between two text entries
    const double framemargin   = 0.2; // Space between text and frame

    // Determine text dimensions
    for(const LegendEntry &entry : this->legend)
    {
        Size labelsize;
        labelsize = this->VectorGraphic::TextBoundingBox(entry.label);
        if(maxtextheight < labelsize.h)
            maxtextheight = labelsize.h;
        if(maxtextwidth < labelsize.w)
            maxtextwidth = labelsize.w;
    }

    // Calculate frame dimensions
    Position legendpos;
    Size     legendsize;
    Size     boxsize;
    boxsize.w    = 1.0;
    boxsize.h    = maxtextheight;
    legendsize.w = maxtextwidth + framemargin * 3 + boxsize.w;
    legendsize.h = (maxtextheight + textmargin) * this->legend.size() - textmargin + framemargin * 2;

    legendpos    = this->histogramposition;
    legendpos.y += this->legendoffset;
    switch(this->legendvalign)
    {
        case VAlign::Left:
            legendpos.x += this->legendoffset;
            break;
        case VAlign::Center:
            legendpos.x += this->histogramsize.w / 2 - legendsize.w / 2;
            break;
        case VAlign::Right:
            legendpos.x += this->histogramsize.w - legendsize.w - this->legendoffset;
            break;
    }

    // Draw Frame
    this->VectorGraphic::Rectangle(legendpos, legendsize, this->legendlinecolor, this->legendbackgroundcolor, this->legendlinewidth);

    // Draw Text
    Position textpos;
    Position boxpos;
    double   deltay;

    boxpos.x = legendpos.x + framemargin;
    boxpos.y = legendpos.y + framemargin;
    textpos.x = boxpos.x + boxsize.w + framemargin;
    textpos.y = boxpos.y + (maxtextheight/2) - 0.02; // -0.02 for visual balancing; Text is center aligned
    deltay    = maxtextheight + textmargin;

    for(const LegendEntry &entry : this->legend)
    {
        Color fg;
        Color bg;
        fg = entry.color;
        bg = entry.color;
        bg.a = 0.5;

        this->VectorGraphic::Rectangle(boxpos, boxsize, fg, bg, this->legendlinewidth);
        this->VectorGraphic::Text(textpos, this->axislegendcolor, entry.label, HAlign::Center, VAlign::Left);

        boxpos.y  += deltay;
        textpos.y += deltay;
    }

    return;
}








