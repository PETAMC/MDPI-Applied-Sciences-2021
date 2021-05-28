#include <vectorgraphic.hpp>
#include <iostream>

VectorGraphic::VectorGraphic(const std::string &filepath, double width, double height)
{
    // size expected in points.
    // 1pt = 1/72.0inch  (Source: [1])
    // 1inch = 2.54cm
    // 1inch = 72pt = 2.54cm -> 1cm = 72pt/2.54cm ≈ 28.346pt
    //  [1] - https://www.cairographics.org/manual/cairo-PDF-Surfaces.html#cairo-pdf-surface-create

    double pdfwidth  = width  * 28.346;
    double pdfheight = height * 28.346;
    this->surface = cairo_pdf_surface_create(filepath.c_str(), pdfwidth, pdfheight);
    this->cairo   = cairo_create(this->surface);

    this->pagesize.w = width;
    this->pagesize.h = height;

    // Find font file for Source Sans Pro
    std::string fontpath;
    if(not this->FindFont("SourceSansPro", &fontpath))
    {
        std::cerr << "Unable to determine path of the SourceSansPro font! Is it installed?\n";
        throw std::runtime_error("Determine SourceSansPro font file failed");
    }

    // Load font
    if(FT_Init_FreeType(&this->ftlib))
    {
        std::cerr << "Initializing FreeType Library failed!";
        throw std::runtime_error("FT_Init_FreeType failed");
    }

    if(FT_New_Face(this->ftlib, fontpath.c_str(), 0, &this->ftface))
    {
        std::cerr << "Loading font \"" << fontpath << "\" failed!";
        throw std::runtime_error("FT_New_Face failed");
    }

    this->cairoftface = cairo_ft_font_face_create_for_ft_face(this->ftface, 0);

    cairo_matrix_init_scale(&this->cairoftmatrix, 1.0, 1.0);
    cairo_matrix_init_identity(&this->cairotm);

    this->cairoftoptions = cairo_font_options_create();
    cairo_font_options_set_hint_style(this->cairoftoptions, CAIRO_HINT_STYLE_FULL);
    cairo_font_options_set_antialias(this->cairoftoptions, CAIRO_ANTIALIAS_BEST);
    
    this->cairofont = cairo_scaled_font_create(this->cairoftface,
                                              &this->cairoftmatrix,
                                              &this->cairotm,
                                              this->cairoftoptions);
}



VectorGraphic::~VectorGraphic()
{
    cairo_destroy(this->cairo);
    cairo_surface_destroy(this->surface);

    cairo_font_options_destroy(this->cairoftoptions);

    // Close font library handler
    static const cairo_user_data_key_t key = {};
    cairo_status_t status = cairo_font_face_set_user_data(this->cairoftface, &key,
                this->ftface, (cairo_destroy_func_t) FT_Done_Face);

    if(status)
    {
        cairo_font_face_destroy(this->cairoftface);
        FT_Done_Face(this->ftface);
    }
}



const Size& VectorGraphic::GetPageSize() const
{
    return this->pagesize;
}



Color VectorGraphic::CreateColor(const std::string &html) const
{
    if(html[0] != '#')
        throw std::invalid_argument("HTML-Notation beginning with \'#\' expected!");

    const char *hexcode = html.c_str() + 1; // skip '#'
    long        bincode = std::strtol(hexcode, nullptr, 16);    // base 16
    long        rcode   = (bincode >> 16) & 0x000000FF;
    long        gcode   = (bincode >>  8) & 0x000000FF;
    long        bcode   = (bincode >>  0) & 0x000000FF;
    double      r       = rcode / 255.0;
    double      g       = gcode / 255.0;
    double      b       = bcode / 255.0;
    return {.r=r, .g=g, .b=b, .a=1.0};
}



void VectorGraphic::Rectangle(const Position &pos, const Size &size, const Color &foreground, const Color &background, double linewidth)
{
    /*
   (x,y)    ├→ path
        ╳   ────────────────────     ┬
        ├┄┄┄┤                        ┆
          r                          ┆
        │                          │ ┆
        │                          │ ┆ h
        │                          │ ┆
        │                          │ ┆
                                     ┆
                                     ┆
            ────────────────────     ┴
        ├┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┄┤
                     w
     */
    // Create rectangle path
    double pi= 3.14159;
    double ph= pi/2.0;
    double x = CM2PT(pos.x) + linewidth/2.0;
    double y = CM2PT(pos.y) + linewidth/2.0;
    double w = CM2PT(size.w)- linewidth;
    double h = CM2PT(size.h)- linewidth;
    double r = CM2PT(0.05);

    cairo_move_to(this->cairo, x+r,       y);                       // Begin path
    cairo_line_to(this->cairo, x+(w-2*r), y);                       // Upper line
    cairo_arc(    this->cairo, x+w-r,     y+r,   r, -ph, 0.0);      // Corner top right
    cairo_line_to(this->cairo, x+w,       y+h-r);                   // Right line
    cairo_arc(    this->cairo, x+w-r,     y+h-r, r, 0.0, ph);       // Corner bottom right
    cairo_line_to(this->cairo, x+r,       y+h);                     // Bottom line
    cairo_arc(    this->cairo, x+r,       y+h-r, r,  ph, pi);       // Corner bottom left
    cairo_line_to(this->cairo, x,         y+r);                     // Left line
    cairo_arc(    this->cairo, x+r,       y+r,   r,  pi, pi+ph);    // Corner top left
    cairo_close_path(this->cairo);                                  // Close path

    // Configure brush
    cairo_set_line_width(this->cairo, linewidth);
    cairo_set_line_cap(this->cairo, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(this->cairo, CAIRO_LINE_JOIN_ROUND);

    // Draw outlining
    cairo_set_source_rgba(this->cairo, COLOR2RGBA(foreground));
    cairo_stroke_preserve(this->cairo);

    // Fill rectangle
    cairo_set_source_rgba(this->cairo, COLOR2RGBA(background));
    cairo_fill(this->cairo);

    return;
}


void VectorGraphic::Line(const Position &start, const Position &end, const Color &color, double linewidth)
{
    cairo_move_to(this->cairo, CM2PT(start.x), CM2PT(start.y)); // Begin path
    cairo_line_to(this->cairo, CM2PT(end.x),   CM2PT(end.y)  ); // End line

    // Configure brush
    cairo_set_line_width(this->cairo, linewidth);
    cairo_set_line_cap(this->cairo, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(this->cairo, CAIRO_LINE_JOIN_ROUND);

    // Draw outlining
    cairo_set_source_rgba(this->cairo, COLOR2RGBA(color));
    cairo_stroke(this->cairo);

    return;
}



void VectorGraphic::DashedLine(const Position &start, const Position &end, const Color &color, double dashlength, double linewidth, double dashoffset)
{
    cairo_save(this->cairo);

    cairo_move_to(this->cairo, CM2PT(start.x), CM2PT(start.y)); // Begin path
    cairo_line_to(this->cairo, CM2PT(end.x),   CM2PT(end.y)  ); // End line

    // Configure brush
    cairo_set_line_width(this->cairo, linewidth);
    cairo_set_line_cap(this->cairo, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(this->cairo, CAIRO_LINE_JOIN_ROUND);
    cairo_set_dash(this->cairo, &dashlength, 1, dashoffset);

    // Draw outlining
    cairo_set_source_rgba(this->cairo, COLOR2RGBA(color));
    cairo_stroke(this->cairo);

    cairo_restore(this->cairo);
    return;
}



void VectorGraphic::Arrow(const Position &start, const Position &end, const Color &color, double linewidth)
{
    Position left, right;
    constexpr const double sideangle  = DEG2RAD(60/2);
              const double sidelength = PT2CM(linewidth * 2);
              const double arrowangle = std::atan2(end.y - start.y, end.x - start.x);

    right.x = end.x - sidelength * std::cos(arrowangle - sideangle);
    right.y = end.y - sidelength * std::sin(arrowangle - sideangle);
    left.x  = end.x - sidelength * std::cos(arrowangle + sideangle);
    left.y  = end.y - sidelength * std::sin(arrowangle + sideangle);

    this->Line(start, end, color, linewidth);
    this->Line(right, end, color, linewidth);
    this->Line(left,  end, color, linewidth);
    return;
}



void VectorGraphic::Text(const Position &pos, const Color &color, const char *line)
{
    //cairo_select_font_face(this->cairo, "cairo:sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_scaled_font(this->cairo, this->cairofont);
    cairo_set_font_size(this->cairo, 10.0);
    cairo_set_source_rgba(this->cairo, COLOR2RGBA(color));

    double x = CM2PT(pos.x);
    double y = CM2PT(pos.y);
    cairo_move_to(this->cairo, x, y);
    cairo_show_text(this->cairo, line);
}


void VectorGraphic::Text(const Position &pos, const Color &color, const std::string &text, HAlign halign, VAlign valign, bool rotate)
{
    const double halfpi = -3.14159/2.0;
    cairo_save(this->cairo);

    cairo_set_scaled_font(this->cairo, this->cairofont);
    cairo_set_font_size(this->cairo, 10.0);
    cairo_set_source_rgba(this->cairo, COLOR2RGBA(color));

    cairo_text_extents_t textextents;
    cairo_text_extents(this->cairo, text.c_str(), &textextents);

    double x = CM2PT(pos.x);
    double y = CM2PT(pos.y);
    
    if(rotate)
    {
        switch(halign)
        {
            case HAlign::Top:
                y += textextents.width;
                break;
            case HAlign::Center:
                y += textextents.width / 2.0;
                break;
            case HAlign::Bottom:
                break;
        }

        switch(valign)
        {
            case VAlign::Right:
                break;
            case VAlign::Center:
                x += textextents.height / 2.0;
                break;
            case VAlign::Left:
                x += textextents.height;
                break;
        }
    }
    else
    {
        switch(halign)
        {
            case HAlign::Top:
                y += textextents.height;
                break;
            case HAlign::Center:
                y += textextents.height / 2.0;
                break;
            case HAlign::Bottom:
                break;
        }

        switch(valign)
        {
            case VAlign::Right:
                x -= textextents.width;
                break;
            case VAlign::Center:
                x -= textextents.width / 2.0;
                break;
            case VAlign::Left:
                break;
        }
    }

    cairo_move_to(this->cairo, x, y);
    if(rotate)
        cairo_rotate(this->cairo, halfpi);
    cairo_show_text(this->cairo, text.c_str());

    cairo_restore(this->cairo);
    return;
}



Size VectorGraphic::TextBoundingBox(const std::string &text)
{
    cairo_text_extents_t textextents;
    cairo_set_scaled_font(this->cairo, this->cairofont);
    cairo_set_font_size(this->cairo, 10.0);
    cairo_text_extents(this->cairo, text.c_str(), &textextents);

    return {.w=PT2CM(textextents.width), .h=PT2CM(textextents.height)};
}



bool VectorGraphic::FindFont(const char* fontname, std::string* path)
{
    // Source: https://stackoverflow.com/questions/10542832/how-to-use-fontconfig-to-get-font-list-c-c

    FcConfig* config = FcInitLoadConfigAndFonts();

    // configure the search pattern, 
    // assume "name" is a std::string with the desired font name in it
    FcPattern* pattern = FcNameParse((const FcChar8*)(fontname));
    FcConfigSubstitute(config, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    // find the font
    FcResult result = FcResultNoMatch; 
    FcPattern* font = FcFontMatch(config, pattern, &result);

    bool retval = false;
    if(font)
    {
        FcChar8* file = NULL;
        if(FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch)
        {
            // save the file to another std::string
            *path  = std::string((char*)file);
            retval = true;
        }
        FcPatternDestroy(font);
    }

    FcPatternDestroy(pattern);
    return retval;
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
