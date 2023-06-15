#include "json.h"
 
using namespace std;
using namespace std::literals;
 
namespace transport_catalogue {
namespace json {
 
namespace {
using namespace std::literals;

Node LoadNode(istream& input);

Node LoadNull(std::istream& input) {
    const string null = "null";
    for (size_t i = 0; i < null.size(); i++) {
        if (null.at(i) == input.get()) continue;
        else throw ParsingError("Failed to parse null");
    }
    return {};
}

Node LoadNumber(std::istream& input) {
    std::string parsed_num;
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
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
const string false_ = "false";
    const string true_ = "true";
    char a = input.get();
    bool value = (a == 't');
    std::string const* name = value ? &true_ : &false_;
    for (size_t i = 1; i < name->size(); i++) {
        if (name->at(i) == input.get()) continue;
        else throw ParsingError("Failed to parse bool");
    }
    return Node(value);
}

std::string LoadString(std::istream& input) {
    using namespace std::literals;
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
    return s;
}

Node LoadArray(std::istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    if (it == end) {
        throw ParsingError("String parsing error");
    }
    Array result;
    for (char c; input >> c && c != ']';) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    return Node(move(result));
}

Node LoadDict(istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    if (it == end) {
        throw ParsingError("String parsing error");
    }
    Dict result;
    for (char c; input >> c && c != '}';) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        if (c == ',') {
            input >> c;
        }
        string key = LoadString(input);
        input >> c;
        result.insert({ move(key), LoadNode(input) });
    }
    return Node(move(result));
}

Node LoadNode(istream& input) {
    char c;
    if (!(input >> c)) {
        throw ParsingError("Failed to parse document"s);
    }
    if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    }
    else if (c == '"') {
        return LoadString(input);
    }
    else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    }
    else if (c == '[') {
        return LoadArray(input);
    }
    else if (c == '{') {
        return LoadDict(input);
    }
    else {
        input.putback(c);
        return LoadNumber(input);
    }
}

}  //namespace

Node::Node(Value value) : std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>(value) {}

int Node::AsInt() const {
    if (!IsInt()) {
        throw std::logic_error(""s);
    }
    return std::get<int>(*this);
}

bool Node::AsBool() const {
    if (!IsBool()) {
        throw std::logic_error(""s);
    }
    return std::get<bool>(*this);
}
   
double Node::AsDouble() const {
    if (!IsDouble()) {
        throw std::logic_error(""s);
    }
    return IsPureDouble() ? std::get<double>(*this) : AsInt();
}

const std::string& Node::AsString() const {
    if (!IsString()) {
        throw std::logic_error(""s);
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
    
bool Node::operator!=(const Node& other) const {
    return !(GetValue() == other.GetValue());
}

bool Document::operator==(const Document& other) const {
    return GetRoot() == other.GetRoot();
}
    
bool Document::operator!=(const Document& other) const {
    return !(GetRoot() == other.GetRoot());
}

std::ostream& operator<<(std::ostream& out, const Node& node) {
    PrintNode(node, out);
    return out;
}

Document::Document() {
    root_ = std::nullptr_t{};
}

Document::Document(Node root): root_(move(root)) {}

const Node& Document::GetRoot() const {
    return root_;
}

Document Load(istream& input) {
    return Document{ LoadNode(input) };
}

PrintContext::PrintContext(std::ostream& out) : out(out) {}

PrintContext::PrintContext(std::ostream& out, int indent_step, int indent) : out(out), indent_step(indent_step), indent(indent) {}

void PrintValue(std::nullptr_t, const PrintContext& text) {
    text.PrintIndent();
    text.out << "null"sv;
}

void PrintValue(std::string str, const PrintContext& text) {
    auto& out = text.out;
    out << "\"";
    for (auto it = str.begin(); it != str.end(); ++it) {
        if (*it == '\\' || *it == '\"' || *it == '\n' || *it == '\t' || *it == '\r') {
            const char escaped_char = *(it);
            switch (escaped_char) {
                case '\n': out << "\\n";
                break;
                case '\t': out << "\t";
                break;
                case '\r': out << "\\r";
                break;
                case '\"': out << "\\\"";
                break;
                case '\\': out << "\\\\";
                break;
                default: throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else {
              out << *it;
          }
    }
    out << "\"";
}

void PrintValue(bool value, const PrintContext& text) {
    auto& out = text.out;
    out << ((value) ? "true"sv : "false"sv);
}

void IdentificationAndPrintValue(const Node::Value& node, const PrintContext& text) {
    if (holds_alternative<std::nullptr_t>(node)) {
        PrintValue(node, text);
    } else if (holds_alternative<int>(node)) {
          PrintValue(std::get<int>(node), text);
      } else if (holds_alternative<double>(node)) {
            PrintValue(std::get<double>(node), text);
        } else if (holds_alternative<bool>(node)) {
              PrintValue(std::get<bool>(node), text);
          } else if (holds_alternative<std::string>(node)) {
                PrintValue(std::get<std::string>(node), text);
            } else if (holds_alternative<Array>(node)) {
                  PrintValue(std::get<Array>(node), text);
              } else {
                    PrintValue(std::get<Dict>(node), text);
                }
}

void PrintValue(const Array& value, const PrintContext& text) {
    auto& out = text.out;
    out << "["sv << std::endl;
    PrintContext text_ = text.Indented();
    for (auto it = value.begin(); it != value.end(); ++it) {
        text_.PrintIndent(); 
        IdentificationAndPrintValue(it->GetValue(), text.Indented());
        if (it != value.end() - 1) {
            out << ", "sv << std::endl;
        }
    }
    out << std::endl;
    text.PrintIndent();
    out << "]"sv;
}

void PrintValue(const Dict& dict, const PrintContext& text) {
    auto& out = text.out;
    out << "{"sv << std::endl;
    PrintContext text_ = text.Indented();
    for (auto it = dict.begin(); it != dict.end(); ++it) {
        text_.PrintIndent();
        out << "\"" << it->first << "\": ";
        IdentificationAndPrintValue(it->second.GetValue(), text_);
        if (it != --dict.end()) {
            out << ", "sv << std::endl;
        }
    }
    out << std::endl;
    text.PrintIndent(); 
    out << "}"sv;
}

void PrintNode(const Node& node, std::ostream& out) {
     std::visit([&out](const auto& value) { PrintValue(value, out); }, node.GetValue());
}

void Print(const Document& doc, std::ostream& output) {
    using namespace std::literals;
    PrintNode(doc.GetRoot(), output);
}
 
}//end namespace json
}//end namespace transport_catalogue
