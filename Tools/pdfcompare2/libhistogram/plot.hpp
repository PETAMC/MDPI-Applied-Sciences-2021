#ifndef PLOT_HPP
#define PLOT_HPP

#include <vectorgraphic.hpp>
#include <histogram.hpp>

struct LegendEntry
{
    LegendEntry(const std::string &label, const Color color)
        : label(label)
        , color(color)
    {
    }
    std::string label;
    Color       color;
};

// ! This class is not thread safe !
class Plot : public VectorGraphic
{
    public:
        Plot(const std::string &path, double width, double height);
        ~Plot();

        void SetLabels(const std::string &xaxis, const std::string &yaxis);
        void SetMaximumMarker(bool enable);
        void SetAverageMarker(bool enable);
        void SetLegendVAlignment(const std::string &alignment);
        void SetLegendVAlignment(VAlign valign);
        void Draw(const std::vector<Histogram> &histograms);

    private:
        void DebugFrame(Position &pos, Size &size);

        void DrawGrid();
        void DrawXAxis();
        void DrawYAxis();
        void DrawXLabels();
        void DrawYLabels();
        void DrawXLegend();
        void DrawYLegend();
        void DrawHistogram(const Histogram &histogram);
        void DrawMaximum(const Histogram &histogram);
        void DrawAverage(const Histogram &histogram);
        void DrawLegend();

        std::vector<LegendEntry> legend;

        long   xmin;
        long   xmax;
        double ymin;
        double ymax;
        double xscale; // \_ cm/value
        double yscale; // /
        std::string xaxislegend;
        std::string yaxislegend;
        bool   drawmaximum;
        bool   drawaverage;

        Color  axiscolor;
        Color  axislabelcolor;
        Color  axislegendcolor;
        Color  gridcolor;
        Color  legendlinecolor;
        Color  legendbackgroundcolor;
        VAlign legendvalign;
        double axislinewidth;
        double gridlinewidth;
        double markerlinewidth;
        double legendlinewidth;
        double legendoffset; // Offset to top, left and/or right edge
        int    xaxisticks;
        int    yaxisticks;

        Position histogramposition;
        Size     histogramsize;

        Position xaxisposition;
        Size     xaxissize;

        Position yaxisposition;
        Size     yaxissize;

        Position xlabelsposition;
        Size     xlabelssize;

        Position ylabelsposition;
        Size     ylabelssize;

        Position xlegendposition;
        Size     xlegendsize;

        Position ylegendposition;
        Size     ylegendsize;

};

#endif

