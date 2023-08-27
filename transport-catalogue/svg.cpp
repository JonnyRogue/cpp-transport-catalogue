#define _USE_MATH_DEFINES
#include "svg.h"

namespace transport_catalogue {
namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, const StrokeLineCap& type_cap) {
    if (type_cap == StrokeLineCap::BUTT) {
        output << "butt";
    } else if (type_cap == StrokeLineCap::ROUND) {
          output << "round";
      } else {
            output << "square";
        }
    return output;
}

std::ostream& operator<<(std::ostream& output, const StrokeLineJoin& type_join) {
    if (type_join == StrokeLineJoin::ARCS) {
        output << "arcs";
    } else if (type_join == StrokeLineJoin::BEVEL) {
          output << "bevel";
      } else if (type_join == StrokeLineJoin::MITER) {
            output << "miter";
        } else if (type_join == StrokeLineJoin::MITER_CLIP) {
              output << "miter-clip";
          } else {
                output << "round";
            }

    return output;
}

void OstreamColorPrinter::operator()(std::monostate) const {
    out << "none";
}

void OstreamColorPrinter::operator()(std::string color) const {
    out << color;
}

void OstreamColorPrinter::operator()(Rgb rgb) const {
    out << "rgb(" << std::to_string(rgb.red)
        << "," << std::to_string(rgb.green)
        << "," << std::to_string(rgb.blue) << ")";
}

void OstreamColorPrinter::operator()(Rgba rgba) const {
    out << "rgba(" << std::to_string(rgba.red)
        << "," << std::to_string(rgba.green)
        << "," << std::to_string(rgba.blue)
        << "," << rgba.opacity << ")";
}

std::ostream& operator<<(std::ostream& output, Color color) {
    std::visit(svg::OstreamColorPrinter{ output }, color);
    return output;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    RenderObject(context);
    context.out << std::endl;
}

Circle& Circle::SetCenter(Point center) {
    center_ = std::move(center);
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(std::move(point));
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    for (auto it = points_.begin(); it != points_.end(); ++it) {
        out << it->x << ","sv << it->y;
        if (it != points_.end() - 1) {
            out << " "sv;
        }
    }
    out << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

Text& Text::SetPosition(Point pos) {
    pos_ = std::move(pos);
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = std::move(offset);
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    name_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text"sv;
    RenderAttrs(context.out);
    out << " x=\""sv << pos_.x << "\" "sv << "y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" "sv << "dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << size_ << "\""sv;
    if (!name_.empty()) {
        out << " font-family=\""sv << name_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">"sv << ConvertTextToSVG() << "<"sv;
    out << "/text>"sv;
}

std::string Text::ConvertTextToSVG() const {
    std::string text = data_;
    ConvertSymbolToSVG(text, '&');
    ConvertSymbolToSVG(text, '<');
    ConvertSymbolToSVG(text, '>');
    ConvertSymbolToSVG(text, '\'');
    ConvertSymbolToSVG(text, '"');
    return text;
}

void Text::ConvertSymbolToSVG(std::string& text, char symbol) const {
    std::string svg_format;
    switch (symbol) {
        case  '&': svg_format = "&amp;";  break;
        case  '<': svg_format = "&lt;";   break;
        case  '>': svg_format = "&gt;";   break;
        case '\'': svg_format = "&apos;"; break;
        case  '"': svg_format = "&quot;"; break;
    }
    size_t count = std::count(text.begin(), text.end(), symbol);
    size_t pos = text.find(symbol);
    size_t i = 1;
    while (i <= count) {
        text = text.replace(pos, 1, svg_format);
        pos = text.find(symbol);
        ++i;
    }
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const auto& object : objects_) {
        out << "  ";
        object->Render(out);
    }
    out << "</svg>"sv;
}
    
} //namespace svg
} //namespace transport_catalogue
