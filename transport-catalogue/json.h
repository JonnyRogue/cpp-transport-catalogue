#pragma once
#include "transport_catalogue.h"

namespace transport_catalogue {
namespace json {
 
using namespace std::literals;

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final
        : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> { 
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
    Value& GetNoConstValue();

};

inline bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

class Document {
public:
    explicit Document();
    explicit Document(Node root);
    const Node& GetRoot() const;
private:
    Node root_;
};

inline bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

inline bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

Document Load(std::istream& input);
    
void Print(const Document& doc, std::ostream& output);
 
}// end namespace json
}// end namespace transport_catalogue
