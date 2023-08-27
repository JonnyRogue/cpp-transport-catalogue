#include "json_builder.h"

namespace transport_catalogue {
namespace json {

Builder::BaseContext::BaseContext(Builder &builder) : builder_(builder) {}

Builder::DictItemContext Builder::BaseContext::StartDict() {
    return builder_.StartDict();
}

Builder::ArrayItemContext Builder::BaseContext::StartArray() {
    return builder_.StartArray();
}

Builder& Builder::BaseContext::EndArray() {
    return builder_.EndArray();
}

Builder& Builder::BaseContext::EndDict() {
    return builder_.EndDict();
}

Builder::KeyItemContext Builder::BaseContext::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& Builder::BaseContext::Value(Node value) {
    return builder_.Value(std::move(value));
}

Builder::KeyItemContext::KeyItemContext(Builder &builder) : BaseContext(builder) {}

Builder::DictItemContext Builder::KeyItemContext::Value(Node value) {
    return DictItemContext(BaseContext::Value(std::move(value)));
}

Builder::DictItemContext::DictItemContext(Builder &builder) : BaseContext(builder) {}

Builder::ArrayItemContext::ArrayItemContext(Builder &builder) : BaseContext(builder) {}

Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node value) {
    return BaseContext::Value(move(value));
}

Builder::Builder() {
    nodes_stack_.emplace_back(&root_);
}

Builder::KeyItemContext Builder::Key(std::string key) {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
        throw std::logic_error("cant build key");
    }
    nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsMap())[key]);
    return *this;
}

Builder& Builder::Value(Node value ) {
    if (NotValidNode()) {
        throw std::logic_error("cant emplace value");
    }
    if (nodes_stack_.back()->IsArray()) {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).emplace_back(value);
    } else {
          *nodes_stack_.back() = value;
          nodes_stack_.pop_back();
      }
    return *this;
}

Builder::DictItemContext Builder::StartDict() {
    Builder::StartContainer(std::move(Dict()));
    return *this;
}

Builder::ArrayItemContext Builder::StartArray() {
    Builder::StartContainer(std::move(Array()));
    return *this;
}

Builder& Builder::EndDict() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
        throw std::logic_error("this node is not Dictionary");
    }
    nodes_stack_.pop_back();
    return *this;
}

Builder& Builder::EndArray() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("this node is not Array");
    }
    nodes_stack_.pop_back();
    return *this;
}

json::Node Builder::Build() {
    if (!nodes_stack_.empty()) {
        throw std::logic_error("cant build node");
    }
    return root_;
}

Builder::~Builder() = default;
    bool Builder::NotValidNode() {
    return (nodes_stack_.empty() || (!nodes_stack_.back()->IsNull() && !nodes_stack_.back()->IsArray()));
}

}//end namespace json
}//end namespace transport_catalogue
