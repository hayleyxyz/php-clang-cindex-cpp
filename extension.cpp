#include "extension.h"

#include <iostream>

namespace php {
namespace clang {
namespace cindex {

Php::Value clang_createIndex(Php::Parameters &params) {
    auto excludeDeclarationsFromPCH = params[0];
    auto displayDiagnostics = params[1];

    auto index = ::clang_createIndex(
            static_cast<bool>(excludeDeclarationsFromPCH),
            static_cast<bool>(displayDiagnostics)
    );

    return Php::Object("CXIndex", new CCXIndex(index));
}

Php::Value clang_parseTranslationUnit(Php::Parameters &params) {
    auto cindexVal = params[0];
    auto cindexContainer = cindexVal.implementation<CCXIndex>();
    auto sourceFilename = params[1];
    auto commandLineArgsParam = params[2];
    auto unusedUnsavedFiles = params[3];
    auto options = params[4].numericValue();

    const char *inputFile = nullptr;
    if(sourceFilename.isString()) {
        inputFile = sourceFilename.stringValue().c_str();
    }

    char **args = nullptr;
    int argsCount = 0;

    if(commandLineArgsParam.isArray() && commandLineArgsParam.length() > 0) {
        args = new char*[commandLineArgsParam.length()];

        for(auto &entry : commandLineArgsParam) {
            auto str = entry.second.stringValue();
            auto s = const_cast<char *>(str.c_str());
            args[argsCount] = new char[str.length() + 1];
            str.copy(args[argsCount], str.length(), 0);
            args[argsCount][str.length()] = '\0';
            argsCount++;
        }
    }

    CXTranslationUnit unit;
    auto err = ::clang_parseTranslationUnit2(
            cindexContainer->getObject(),
            inputFile,
            args, argsCount,
            nullptr, 0,
            options,
            &unit
    );

    for(int i = 0; i < argsCount; i++) {
        delete[] args[i];
    }

    delete[] args;

    if (err != CXError_Success) {
        throw Php::Exception("clang_parseTranslationUnit2() failed");
    }

    return Php::Object("CXTranslationUnit", new CCXTranslationUnit(unit));
}

Php::Value clang_getTranslationUnitCursor(Php::Parameters &params) {
    auto translationUnitContainer = params[0].implementation<CCXTranslationUnit>();

    auto cursor = ::clang_getTranslationUnitCursor(translationUnitContainer->getObject());

    return Php::Object("CXCursor", new CCXCursor(cursor));
}

CXChildVisitResult cursorVisitor(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    Php::Value *value = static_cast<Php::Value*>(client_data);

    Php::Value result = (*value)(
            Php::Object("CXCursor", new CCXCursor(cursor)),
            Php::Object("CXCursor", new CCXCursor(parent))
    );

    if(result.isNumeric()) {
        switch(result.numericValue()) {
            case CXChildVisit_Break:
            case CXChildVisit_Continue:
            case CXChildVisit_Recurse:
                return static_cast<CXChildVisitResult>(result.numericValue());
        }
    }

    return CXChildVisitResult::CXChildVisit_Break;
}

Php::Value clang_visitChildren(Php::Parameters &params) {
    auto cursorContainer = params[0].implementation<CCXCursor>();
    auto closure = params[1];

    return static_cast<int32_t>(::clang_visitChildren(cursorContainer->getObject(), cursorVisitor, &closure));
}

Php::Value clang_getCursorSpelling(Php::Parameters &params) {
    return simple_container_func<CCXCursor, CXCursor, CXString>(params, ::clang_getCursorSpelling);
}

Php::Value clang_getCursorExtent(Php::Parameters &params) {
    auto cursorContainer = params[0].implementation<CCXCursor>();

    auto extent = ::clang_getCursorExtent(cursorContainer->getObject());

    return Php::Object("CXSourceRange", new CCXSourceRange(extent));
}

Php::Value clang_getRangeStart(Php::Parameters &params) {
    auto sourceRangeContainer = params[0].implementation<CCXSourceRange>();

    auto location = ::clang_getRangeStart(sourceRangeContainer->getObject());

    return Php::Object("CXSourceLocation", new CCXSourceLocation(location));
}

Php::Value clang_getRangeEnd(Php::Parameters &params) {
    auto sourceRangeContainer = params[0].implementation<CCXSourceRange>();
    auto location = ::clang_getRangeEnd(sourceRangeContainer->getObject());
    return Php::Object("CXSourceLocation", new CCXSourceLocation(location));
}

void clang_getInstantiationLocation(Php::Parameters &params) {
    auto sourceLocationContainer = params[0].implementation<CCXSourceLocation>();

    CXFile file { nullptr };
    unsigned line = 0, column = 0, offset = 0;

    ::clang_getInstantiationLocation(
            sourceLocationContainer->getObject(),
            &file,
            &line,
            &column,
            &offset
    );

    if(file == nullptr) {
        params[1] = nullptr;
    }
    else {
        auto fileObject = Php::Object("CXFile", new CCXFile(file));
        params[1] = fileObject;
    }

    params[2] = static_cast<int32_t>(line);
    params[3] = static_cast<int32_t>(column);
    params[4] = static_cast<int32_t>(offset);
}

Php::Value clang_getFileName(Php::Parameters &params) {
    return simple_container_func<CCXFile, CXFile, CXString>(params, ::clang_getFileName);
}

Php::Value clang_getCursorUSR(Php::Parameters &params) {
    return simple_container_func<CCXCursor, CXCursor, CXString>(params, ::clang_getCursorUSR);
}

Php::Value clang_getCursorKind(Php::Parameters &params) {
    return simple_container_func<CCXCursor, CXCursor, CXCursorKind>(params, ::clang_getCursorKind);
}

Php::Value clang_getCursorKindSpelling(Php::Parameters &params) {
    return Value(clang_getCursorKindSpelling(
            static_cast<CXCursorKind>(params[0].numericValue())
    ));
}

Php::Value clang_getCursorType(Php::Parameters &params) {
    auto cursorContainer = params[0].implementation<CCXCursor>();

    auto type = ::clang_getCursorType(cursorContainer->getObject());
    return Php::Object("CXType", new CCXType(type));
}

Php::Value clang_getTypeSpelling(Php::Parameters &params) {
    return simple_container_func<CCXType, CXType, CXString>(params, ::clang_getTypeSpelling);
}

Php::Value clang_getCursorDisplayName(Php::Parameters &params) {
    return simple_container_func<CCXCursor, CXCursor, CXString>(params, ::clang_getCursorDisplayName);
}

Php::Value clang_isDeclaration(Php::Parameters &params) {
    return simple_int_func<CXCursorKind, unsigned int, bool>(params, ::clang_isDeclaration);
}

Php::Value clang_isReference(Php::Parameters &params) {
    return simple_int_func<CXCursorKind, unsigned int, bool>(params, ::clang_isReference);
}

Php::Value clang_isExpression(Php::Parameters &params) {
    return simple_int_func<CXCursorKind, unsigned int, bool>(params, ::clang_isExpression);
}

Php::Value clang_isStatement(Php::Parameters &params) {
    return simple_int_func<CXCursorKind, unsigned int, bool>(params, ::clang_isStatement);
}

Php::Value clang_isAttribute(Php::Parameters &params) {
    return simple_int_func<CXCursorKind, unsigned int, bool>(params, ::clang_isAttribute);
}

Php::Value clang_isInvalid(Php::Parameters &params) {
    return simple_int_func<CXCursorKind, unsigned int, bool>(params, ::clang_isInvalid);
}

Php::Value clang_isTranslationUnit(Php::Parameters &params) {
    return simple_int_func<CXCursorKind, unsigned int, bool>(params, ::clang_isTranslationUnit);
}

Php::Value clang_isPreprocessing(Php::Parameters &params) {
    return simple_int_func<CXCursorKind, unsigned int, bool>(params, ::clang_isPreprocessing);
}

Php::Value clang_isUnexposed(Php::Parameters &params) {
    return simple_int_func<CXCursorKind, unsigned int, bool>(params, ::clang_isUnexposed);
}

Php::Value clang_getCursorLinkage(Php::Parameters &params) {
    return simple_container_func<CCXCursor, CXCursor, CXLinkageKind>(params, ::clang_getCursorLinkage);
}

Php::Value clang_getCursorVisibility(Php::Parameters &params) {
    return simple_container_func<CCXCursor, CXCursor, CXVisibilityKind>(params, ::clang_getCursorVisibility);
}

Php::Value clang_getCursorAvailability(Php::Parameters &params) {
    return simple_container_func<CCXCursor, CXCursor, CXAvailabilityKind>(params, ::clang_getCursorAvailability);
}

Php::Value clang_getCursorLanguage(Php::Parameters &params) {
    return simple_container_func<CCXCursor, CXCursor, CXLanguageKind>(params, ::clang_getCursorLanguage);
}

Php::Value clang_getCursorTLSKind(Php::Parameters &params) {
    return simple_container_func<CCXCursor, CXCursor, CXTLSKind>(params, ::clang_getCursorTLSKind);
}

extern "C"
{
PHPCPP_EXPORT void *get_module() {
    static Php::Extension extension("php-clang-cpp", "1.0");

    extension.add<clang_createIndex>("clang_createIndex", {
            Php::ByVal("excludeDeclarationsFromPCH", Php::Type::Bool),
            Php::ByVal("displayDiagnostics", Php::Type::Bool)
    });

    extension.add<clang_parseTranslationUnit>("clang_parseTranslationUnit", {
            Php::ByVal("CIdx", "CXIndex"),
            Php::ByVal("source_filename"),
            Php::ByVal("command_line_args"),
            Php::ByVal("unimplemented_unsaved_files"),
            Php::ByVal("options", Php::Type::Numeric),
    });

    extension.add<clang_getTranslationUnitCursor>("clang_getTranslationUnitCursor", {
            Php::ByVal("translationUnit", "CXTranslationUnit"),
    });

    extension.add<clang_visitChildren>("clang_visitChildren", {
            Php::ByVal("parent", "CXCursor"),
            Php::ByVal("visitor", Php::Type::Callable)
    });

    extension.add<clang_getCursorSpelling>("clang_getCursorSpelling", {
            Php::ByVal("cursor", "CXCursor")
    });

    extension.add<clang_getCursorExtent>("clang_getCursorExtent", {
            Php::ByVal("cursor", "CXCursor")
    });

    extension.add<clang_getRangeStart>("clang_getRangeStart", {
            Php::ByVal("range", "CXSourceRange")
    });

    extension.add<clang_getRangeEnd>("clang_getRangeEnd", {
            Php::ByVal("range", "CXSourceRange")
    });

    extension.add<clang_getInstantiationLocation>("clang_getInstantiationLocation", {
            Php::ByVal("location", "CXSourceLocation"),
            Php::ByRef("file"),
            Php::ByRef("line"),
            Php::ByRef("column"),
            Php::ByRef("offset")
    });

    extension.add<clang_getFileName>("clang_getFileName", {
            Php::ByVal("file", "CXFile")
    });

    extension.add<clang_getCursorUSR>("clang_getCursorUSR", {
            Php::ByVal("cursor", "CXCursor")
    });

    extension.add<clang_getCursorKind>("clang_getCursorKind", {
            Php::ByVal("cursor", "CXCursor")
    });

    extension.add<clang_getCursorKindSpelling>("clang_getCursorKindSpelling", {
            Php::ByVal("kind", Php::Type::Numeric)
    });

    extension.add<clang_getCursorType>("clang_getCursorType", {
            Php::ByVal("cursor", "CXCursor")
    });

    extension.add<clang_getTypeSpelling>("clang_getTypeSpelling", {
            Php::ByVal("type", "CXType")
    });

    extension.add<clang_getCursorDisplayName>("clang_getCursorDisplayName", {
            Php::ByVal("cursor", "CXCursor")
    });

    extension.add<clang_isDeclaration>("clang_isDeclaration", {
            Php::ByVal("kind", Php::Type::Numeric)
    });

    extension.add<clang_isReference>("clang_isReference", {
            Php::ByVal("kind", Php::Type::Numeric)
    });

    extension.add<clang_isExpression>("clang_isExpression", {
            Php::ByVal("kind", Php::Type::Numeric)
    });

    extension.add<clang_isStatement>("clang_isStatement", {
            Php::ByVal("kind", Php::Type::Numeric)
    });

    extension.add<clang_isAttribute>("clang_isAttribute", {
            Php::ByVal("kind", Php::Type::Numeric)
    });

    extension.add<clang_isInvalid>("clang_isInvalid", {
            Php::ByVal("kind", Php::Type::Numeric)
    });

    extension.add<clang_isTranslationUnit>("clang_isTranslationUnit", {
            Php::ByVal("kind", Php::Type::Numeric)
    });

    extension.add<clang_isPreprocessing>("clang_isPreprocessing", {
            Php::ByVal("kind", Php::Type::Numeric)
    });

    extension.add<clang_isUnexposed>("clang_isUnexposed", {
            Php::ByVal("kind", Php::Type::Numeric)
    });

    extension.add<clang_getCursorLinkage>("clang_getCursorLinkage", {
            Php::ByVal("cursor", "CXCursor")
    });

    extension.add<clang_getCursorVisibility>("clang_getCursorVisibility", {
            Php::ByVal("cursor", "CXCursor")
    });

    extension.add<clang_getCursorAvailability>("clang_getCursorAvailability", {
            Php::ByVal("cursor", "CXCursor")
    });

    extension.add<clang_getCursorLanguage>("clang_getCursorLanguage", {
            Php::ByVal("cursor", "CXCursor")
    });

    extension.add<clang_getCursorTLSKind>("clang_getCursorTLSKind", {
            Php::ByVal("cursor", "CXCursor")
    });

    Php::Class<CCXIndex> CXIndex("CXIndex");
    extension.add(std::move(CXIndex));

    Php::Class<CCXTranslationUnit> CXTranslationUnit("CXTranslationUnit");
    extension.add(std::move(CXTranslationUnit));

    Php::Class<CCXCursor> CXCursor("CXCursor");
    extension.add(std::move(CXCursor));

    Php::Class<CCXSourceRange> CXSourceRange("CXSourceRange");
    extension.add(std::move(CXSourceRange));

    Php::Class<CCXSourceLocation> CXSourceLocation("CXSourceLocation");
    extension.add(std::move(CXSourceLocation));

    Php::Class<CCXFile> CXFile("CXFile");
    extension.add(std::move(CXFile));

    Php::Class<CCXType> CXType("CXType");
    CXType.property("kind", &CCXType::getKind);
    extension.add(std::move(CXType));

    return extension.module();
}
}

}
}
}