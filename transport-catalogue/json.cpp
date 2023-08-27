#include "json.h"
 
using namespace std;
using namespace std::literals;
 
namespace transport_catalogue {
namespace json {
 
namespace {
using namespace std::literals;

Node LoadNode(istream& input);
Node LoadString(std::istream& input);

std::string LoadLiteral(std::istream& input) {
    std::string s;
    while (std::isalpha(input.peek())) {
        s.push_back(static_cast<char>(input.get()));
    }
    return s;
}
    
Node LoadNull(std::istream& input) {
    if (auto literal = LoadLiteral(input); literal == "null"sv) {
        return Node{ nullptr };
    } else {
          throw ParsingError("Failed to parse '"s + literal + "' as null"s);
      }
}

Node LoadNumber(std::istream& input) {
    std::string parsed_num;
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number"s);
        }
    };
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };
    if (input.peek() == '-') {
        read_char();
    }
    if (input.peek() == '0') {
        read_char();
    } else {
          read_digits();
      }
    bool is_int = true;
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }
    try { 
        if (is_int) {
            try {
                return std::stoi(parsed_num);
            }
            catch (...) {
            }
        }
        return std::stod(parsed_num);
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadBool(std::istream& input) {
    const auto s = LoadLiteral(input);
    if (s == "true"sv) {
        return Node{ true };
    }
    else if (s == "false"sv) {
                return Node{ false };
    } else {
          throw ParsingError("Failed to parse '"s + s + "' as bool"s);
      }
}

Node LoadString(std::istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
        } else if (ch == '\\') {
              ++it;
              if (it == end) {
                  throw ParsingError("String parsing error");
              }
              const char escaped_char = *(it);
              switch (escaped_char) {
                  case 'n':
                  s.push_back('\n');
                  break;
                  case 't':
                  s.push_back('\t');
                  break;
                  case 'r':
                  s.push_back('\r');
                  break;
                  case '"':
                  s.push_back('"');
                  break;
                  case '\\':
                  s.push_back('\\');
                  break;
                  default:
                  throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
              }
          } else if (ch == '\n' || ch == '\r') {
                throw ParsingError("Unexpected end of line"s);
            } else {
                  s.push_back(ch);
              }
              ++it;
    }
    return Node(std::move(s));
}

Node LoadArray(std::istream& input) {
    std::vector<Node> result;
    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (!input) {
        throw ParsingError("Array parsing error"s);
    }
    return Node(std::move(result));
}

Node LoadDict(istream& input) {
    Dict dict;
    for (char c; input >> c && c != '}';) {
        if (c == '"') {
            std::string key = LoadString(input).AsString();
            if (input >> c && c == ':') {
                if (dict.find(key) != dict.end()) {
                    throw ParsingError("Duplicate key '"s + key + "' have been found");
                }
                dict.emplace(std::move(key), LoadNode(input));
            } else {
                  throw ParsingError(": is expected but '"s + c + "' has been found"s);
              }
        }
        else if (c != ',') {
            throw ParsingError(R"(',' is expected but ')"s + c + "' has been found"s);
        }
    }
    if (!input) {
        throw ParsingError("Dictionary parsing error"s);
    }
    return Node(std::move(dict));
}

Node LoadNode(istream& input) {
    char c;
    if (!(input >> c)) {
        throw ParsingError("Unexpected EOF"s);
    }
    switch (c) {
        case '[' : return LoadArray(input);
        case '{' : return LoadDict(input);
        case '"' : return LoadString(input);
        case 't' : [[fallthrough]];
        case 'f' : input.putback(c);
                   return LoadBool(input);
        case 'n' : input.putback(c);
                   return LoadNull(input);
        default : input.putback(c);
                  return LoadNumber(input);
    }
}  

struct PrintContext {
    std::ostream& out;
    int indention = 4;
    int indention_ = 0;
    void PrintIndent() const {
        for (int i = 0; i < indention_; ++i) {
            out.put(' ');
        }
    }
    PrintContext Indented() const {
        return { out, indention, indention + indention_ };
    }
};
    
void PrintNode(const Node& value, const PrintContext& context);

template <typename Value>
void PrintValue(const Value& value, const PrintContext& context) {
    context.out << value;
}

void PrintString(const std::string& value, std::ostream& out) {
    out.put('"');
    for (const char c : value) {
        switch (c) {
            case '\r' : out << "\\r"sv;
            break;
            case '\n' : out << "\\n"sv;
            break;
            case '"' : [[fallthrough]];
            case '\\' : out.put('\\');
                        [[fallthrough]];
            default : out.put(c);
                      break;
        }
    }
    out.put('"');
}

template <>
void PrintValue<std::string>(const std::string& value, const PrintContext& context) {
    PrintString(value, context.out);
}

template <>
void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& context) {
    context.out << "null"sv;
}

template <>
void PrintValue<bool>(const bool& value, const PrintContext& context) {
    context.out << (value ? "true"sv : "false"sv);
}

template <>
void PrintValue<Array>(const Array& nodes, const PrintContext& context) {
    std::ostream& out = context.out;
    out << "[\n"sv;
    bool first = true;
    auto inner_context = context.Indented();
    for (const Node& node : nodes) {
        if (first) {
            first = false;
        } else {
              out << ",\n"sv;
          }
          inner_context.PrintIndent();
          PrintNode(node, inner_context);
    }
    out.put('\n');
    context.PrintIndent();
    out.put(']');
}

template <>
void PrintValue<Dict>(const Dict& nodes, const PrintContext& context) {
    std::ostream& out = context.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_context = context.Indented();
    for (const auto& [key, node] : nodes) {
        if (first) {
            first = false;
        } else {
              out << ",\n"sv;
          }
          inner_context.PrintIndent();
          PrintString(key, context.out);
          out << ": "sv;
          PrintNode(node, inner_context);
    }
    out.put('\n');
    context.PrintIndent();
    out.put('}');
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit( 
        [&ctx](const auto& value) {
            PrintValue(value, ctx);
        }, node.GetValue());
}
}
    
int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error("Not int"s);
    }
    return std::get<int>(*this);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error("Not bool"s);
    }
    return std::get<bool>(*this);
}
   
double Node::AsDouble() const {
    if (!IsDouble()) {
        throw std::logic_error("Not double"s);
    }
    return IsPureDouble() ? std::get<double>(*this) : AsInt();
}

const std::string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error("Not string"s);
    }
    return std::get<std::string>(*this);
}

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw std::logic_error("Not an array"s);
    }
    return std::get<Array>(*this);
}   
    
const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw std::logic_error("Not a dict"s);
    }
    return std::get<Dict>(*this);
}

bool Node::IsInt() const {
    return holds_alternative<int>(*this);
}

bool Node::IsPureDouble() const {
    return holds_alternative<double>(*this);
}

bool Node::IsDouble() const {
    return IsInt() || IsPureDouble();
}
    
bool Node::IsBool() const {
    return holds_alternative<bool>(*this);
}
   
bool Node::IsString() const {
    return holds_alternative<std::string>(*this);
}
   
bool Node::IsNull() const {
    return holds_alternative<std::nullptr_t>(*this);
}
    
bool Node::IsArray() const {
    return holds_alternative<Array>(*this);
}
    
bool Node::IsMap() const {
    return holds_alternative<Dict>(*this);
}

const Node::Value& Node::GetValue() const {
    return *this;
}

bool Node::operator==(const Node& other) const {
    return GetValue() == other.GetValue();
}
    
Node::Value& Node::GetNoConstValue() {
    return *this;
}

Document Load(istream& input) {
    return Document{ LoadNode(input) };
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext{ output });
}
 
}//end namespace json
}//end namespace transport_catalogue
