#ifndef PHP_CLANG_CINDEX_LIBRARY_H
#define PHP_CLANG_CINDEX_LIBRARY_H

#include <phpcpp.h>
#include <clang-c/Index.h>

namespace php {
namespace clang {
namespace cindex {

/**
 * Container object to wrap all of Clang CIndex's gubbins
 * @tparam T
 */
template <typename T>
class Container : public ::Php::Base {
private:
    T object;

public:
    Container() { }
    Container(T object) : object(object) { }
    virtual ~Container() { }

    void setObject(T object) {
        this->object = object;
    }

    T getObject() {
        return this->object;
    }

    explicit operator T() {
        return this->getObject();
    }
};

class CCXType : public Container<CXType> {
public:
    CCXType() { }
    CCXType(CXType object) : Container(object) { }

    Php::Value getKind() {
        return this->getObject().kind;
    }
};

using CCXFile = Container<CXFile>;
using CCXSourceLocation = Container<CXSourceLocation>;
using CCXSourceRange = Container<CXSourceRange>;
using CCXIndex = Container<CXIndex>;
using CCXTranslationUnit = Container<CXTranslationUnit>;
using CCXCursor = Container<CXCursor>;

class Value : public ::Php::Value {
public:
    Value(const CXString& from) : ::Php::Value(clang_getCString(from)) {
        clang_disposeString(from);
    }

    Value(const int32_t& from) : ::Php::Value(from) { }
    Value(const int64_t& from) : ::Php::Value(from) { }
    Value(const Php::Value& from) : ::Php::Value(from) { }

    explicit operator CXCursorKind() const {
        return static_cast<CXCursorKind>(this->numericValue());
    }
};

template <typename php_val, typename input_val, typename return_val>
Value simple_container_func(Php::Parameters &params, return_val (*func_ptr)(input_val)) {
    auto container = params[0].implementation<php_val>();
    return func_ptr(container->getObject());
}

template <typename input_val, typename clang_return_val, typename php_return_val>
Php::Value simple_int_func(Php::Parameters &params, clang_return_val (*func_ptr)(input_val)) {
    auto cursorKind = static_cast<input_val>(Value(params[0]));
    auto result = func_ptr(cursorKind);
    return static_cast<php_return_val>(result);
}

}
}
}

#endif