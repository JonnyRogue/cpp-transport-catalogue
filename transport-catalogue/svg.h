#pragma once
#include "transport_catalogue.h"

namespace transport_catalogue {
namespace svg {

enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};

enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

std::ostream& operator<<(std::ostream& output, const StrokeLineCap& type_cap);
std::ostream& operator<<(std::ostream& output, const StrokeLineJoin& type_join);

class Rgb {
public:
    Rgb() = default;
    Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}
    
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

class Rgba {
public:
    Rgba() = default;
    Rgba(uint8_t r, uint8_t g, uint8_t b, double o) : red(r), green(g), blue(b), opacity(o) {}

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{ "none" };

struct OstreamColorPrinter {
    std::ostream& out;
    void operator()(std::monostate) const;
    void operator()(std::string color) const;
    void operator()(Rgb rgb) const;
    void operator()(Rgba rgba) const;
};

std::ostream& operator<<(std::ostream& output, Color color);

struct Point {
    Point() = default;
    Point(double x, double y) : x(x), y(y) {}
    
    double x = 0;
    double y = 0;
};

struct RenderContext {
    RenderContext(std::ostream& out) : out(out) {}

    RenderContext(std::ostream& out, int indent_step, int indent = 0) : out(out), indent_step(indent_step), indent(indent) {}

    RenderContext Indented() const {
        return { out, indent_step, indent + indent_step };
    }
    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }
    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color);
    Owner& SetStrokeColor(Color color);
    Owner& SetStrokeWidth(double width);
    Owner& SetStrokeLineCap(StrokeLineCap line_cap);
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join);
protected:
    ~PathProps() = default;
    void RenderAttrs(std::ostream& out) const;
private:
    Owner& AsOwner();
    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
};

class Object {
public:
    void Render(const RenderContext& context) const;
    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

class Circle final : public Object, public PathProps<Circle> {
public:
    Circle& SetCenter(Point center);
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;
    Point center_;
    double radius_ = 1.0;
};

class Polyline final : public Object, public PathProps<Polyline> {
public:
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;
    std::vector<Point> points_;
};

class Text final : public Object, public PathProps<Text> {
public:
    Text& SetPosition(Point pos);
    Text& SetOffset(Point offset);
    Text& SetFontSize(uint32_t size);
    Text& SetFontFamily(std::string font_family);
    Text& SetFontWeight(std::string font_weight);
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;
    std::string ConvertTextToSVG() const;
    void ConvertSymbolToSVG(std::string& text, char symbol) const;
    Point pos_;
    Point offset_;
    uint32_t size_ = 1;
    std::string name_;
    std::string font_weight_;
    std::string data_;
};

class ObjectContainer {
public:
    template <typename T>
    void Add(T obj);
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;
    virtual ~ObjectContainer() = default;
};

template <typename T>
void ObjectContainer::Add(T obj) {
    AddPtr(std::make_unique<T>(std::move(obj)));
}

class Document : public ObjectContainer {
public:
    void AddPtr(std::unique_ptr<Object>&& obj) override;
    void Render(std::ostream& out) const;

private:
    std::vector<std::unique_ptr<Object>> objects_;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;
    virtual ~Drawable() = default;
};

template <typename Owner>
Owner& PathProps<Owner>::SetFillColor(Color color) {
    fill_color_ = color;
    return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeColor(Color color) {
    stroke_color_ = std::move(color);
    return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeWidth(double width) {
    stroke_width_ = width;
    return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeLineCap(StrokeLineCap line_cap) {
    stroke_line_cap_ = line_cap;
    return AsOwner();
}

template <typename Owner>
Owner& PathProps<Owner>::SetStrokeLineJoin(StrokeLineJoin line_join) {
    stroke_line_join_ = line_join;
    return AsOwner();
}

template <typename Owner>
void PathProps<Owner>::RenderAttrs(std::ostream& out) const {
    using namespace std::literals;
    if (fill_color_) {
        out << " fill=\""sv;
        std::visit(OstreamColorPrinter{ out }, *fill_color_);
        out << "\""sv;
    }
    if (stroke_color_) {
        out << " stroke=\""sv;
        std::visit(OstreamColorPrinter{ out }, *stroke_color_);
        out << "\""sv;
    }
    if (stroke_width_) {
        out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
    }
    if (stroke_line_cap_) {
        out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
    }
    if (stroke_line_join_) {
        out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
    }
}

template <typename Owner>
Owner& PathProps<Owner>::AsOwner() {
    return static_cast<Owner&>(*this);
}

} //namespace svg 
} //namespace transport_catalogue
