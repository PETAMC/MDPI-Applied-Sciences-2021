#ifndef VECTORGRAPHIC_HPP
#define VECTORGRAPHIC_HPP

#include <cairo-pdf.h>
#include <cairo-ft.h>
#include <tuple>
#include <string>
#include <cmath>

#include <ft2build.h>
#include FT_FREETYPE_H

struct Position
{
    double x;
    double y;
}; // in cm

struct Size
{
    double w;
    double h;
}; // in cm

struct Color
{
    double r;
    double g;
    double b;
    double a;
}; // 0.0 .. 1.0

#define CM2PT(cm) (cm * 28.346)
#define PT2CM(pt) (pt / 28.346)
#define COLOR2RGB(c) c.r, c.g, c.b
#define COLOR2RGBA(c) c.r, c.g, c.b, c.a
#define DEG2RAD(d) (d * M_PI / 180.0)
// r = d° × π/180

enum class HAlign
{
    Top, Center, Bottom
};
enum class VAlign
{
    Left, Center, Right
};

class VectorGraphic
{
    public:
        // all sizes and positions in cm
        VectorGraphic(const std::string &filepath, double width, double height);
        ~VectorGraphic();

        Color CreateColor(const std::string &html) const;

        const Size& GetPageSize() const;

        void Text(const Position &pos, const Color &color, const char *line);
        void Text(const Position &pos, const Color &color, const std::string &text, HAlign halign, VAlign valign, bool rotate=false);
        Size TextBoundingBox(const std::string &text);
        void Rectangle(const Position &pos, const Size &size, const Color &foreground, const Color &background, double linewidth=2.3);
        void Line(const Position &start, const Position &end, const Color &color, double linewidth=2.3);
        void DashedLine(const Position &start, const Position &end, const Color &color, double dashlength=1.0, double linewidth=2.3, double dashoffset=0.0);
        void Arrow(const Position &start, const Position &end, const Color &color, double linewidth=2.3);

    protected:
        Size            pagesize;

    private:
        bool FindFont(const char* fontname, std::string* path);

        cairo_surface_t *surface;
        cairo_t         *cairo;

        cairo_matrix_t     cairoftmatrix;
        cairo_matrix_t     cairotm;
        cairo_font_options_t* cairoftoptions;
        cairo_font_face_t *cairoftface;
        cairo_scaled_font_t *cairofont;

        FT_Library      ftlib;
        FT_Face         ftface;
};

#endif

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
