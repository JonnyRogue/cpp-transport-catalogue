#pragma once
#include "transport_catalogue.h"

namespace transport_catalogue {
namespace json {
 
using namespace std::literals;

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;
    
    PrintContext(std::ostream& out);
    
    PrintContext(std::ostream& out, int indent_step, int indent = 0);
    
    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }
    
    PrintContext Indented() const {
        return { out, indent_step, indent_step + indent };
    }
};

template <typename Value>
void PrintValue(const Value& value, const PrintContext& text) {
    text.out << value;
}

void PrintValue(std::nullptr_t, const PrintContext& text);

void PrintValue(bool value, const PrintContext& text);

void PrintValue(const Array& value, const PrintContext& text);

void PrintValue(const Dict& value, const PrintContext& text);

void PrintNode(const Node& node, std::ostream& out);

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final
        : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> { // достаточно ли этого? или вы имели ввиду что-то другое?
public:
    using variant::variant;
    using Value = variant;
    Node(Value value);
            
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    const Value& GetValue() const;
    bool operator==(const Node& other) const;
    bool operator!=(const Node& other) const;

};

std::ostream& operator<<(std::ostream& out, const Node& node);

class Document {
public:
    explicit Document();
    explicit Document(Node root);
    const Node& GetRoot() const;
    bool operator==(const Document& other) const;
    bool operator!=(const Document& other) const;

private:
    Node root_;
};

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);
    
 
}// end namespace json
}// end namespace transport_catalogue
