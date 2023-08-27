#pragma once
 
#include "json.h"

namespace transport_catalogue {
namespace json {
 
class Builder {
private:
    class DictItemContext;
    class KeyItemContext;
    class ArrayItemContext;
    
    class BaseContext {
    public:
        BaseContext(Builder& builder);
        DictItemContext StartDict();
        ArrayItemContext StartArray();
        KeyItemContext Key(std::string key);
        Builder& EndDict();
        Builder& EndArray();
        Builder& Value(json::Node value);
    private:
        Builder& builder_;
    };
    
    class KeyItemContext : public BaseContext {
    public:
        KeyItemContext(Builder& builder);
        Builder& EndDict() = delete;
        Builder& EndArray() = delete;
        KeyItemContext Key(std::string key) = delete;
        DictItemContext Value(Node value);
    };
    
    class DictItemContext : public BaseContext {
    public:
        DictItemContext(Builder& builder);
        DictItemContext StartDict() = delete;
        ArrayItemContext StartArray() = delete;
        Builder& EndArray() = delete;
        Builder& Value(Node value) = delete;
    };
    
    class ArrayItemContext : public BaseContext {
    public:
        ArrayItemContext(Builder& builder);
        KeyItemContext Key(std::string key) = delete;
        ArrayItemContext Value(Node value);
        Builder& EndDict() = delete;
    };

public:
    Builder();
    KeyItemContext Key(std::string string);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& Value(Node value);
    Builder& EndDict();
    Builder& EndArray();
    json::Node Build();
    ~Builder();
private:
    Node root_;
    std::vector<Node*> nodes_stack_;
    bool NotValidNode();
    template<typename Container>
    void StartContainer(Container container);
};

template<typename Container>
void Builder::StartContainer(Container container) {
    if (NotValidNode()) {
        throw std::logic_error("cant start Array");
    }
    if (nodes_stack_.back()->IsArray()) {
        nodes_stack_.emplace_back(&const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(container));
    } else {
          *nodes_stack_.back() = container;
      }
}
 
}//end namespace json
}//end namespace transport_catalogue
