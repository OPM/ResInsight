#pragma once

#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmXmlStringValidation.h"

// Taken from gtest.h
//
// Due to C++ preprocessor weirdness, we need double indirection to
// concatenate two tokens when one of them is __LINE__.  Writing
//
//   foo ## __LINE__
//
// will result in the token foo__LINE__, instead of foo followed by
// the current line number.  For more details, see
// http://www.parashift.com/c++-faq-lite/misc-technical-issues.html#faq-39.6
#define PDM_OBJECT_STRING_CONCATENATE(foo, bar) PDM_OBJECT_STRING_CONCATENATE_IMPL_(foo, bar)
#define PDM_OBJECT_STRING_CONCATENATE_IMPL_(foo, bar) foo ## bar

#define CAF_PDM_VERIFY_XML_KEYWORD(keyword) \
        static_assert(isFirstCharacterValidInXmlKeyword(keyword),   "First character in keyword is invalid"); \
        static_assert(!isFirstThreeCharactersXml(keyword),          "Keyword starts with invalid sequence xml"); \
        static_assert(isValidXmlKeyword(keyword),                   "Detected invalid character in keyword");


/// CAF_PDM_HEADER_INIT assists the factory used when reading objects from file
/// Place this in the header file inside the class definition of your PdmObject

// To be renamed CAF_PDM_XML_HEADER_INIT
#define CAF_PDM_XML_HEADER_INIT \
public: \
    virtual QString             classKeyword() const; \
    static QString              classKeywordStatic(); \
    static std::vector<QString> classKeywordAliases(); \
    virtual bool                matchesClassKeyword(const QString& keyword) const; \
    \
    static  bool Error_You_forgot_to_add_the_macro_CAF_PDM_XML_HEADER_INIT_and_or_CAF_PDM_XML_SOURCE_INIT_to_your_cpp_file_for_this_class()

#define CAF_PDM_XML_ABSTRACT_SOURCE_INIT(ClassName, keyword, ...) \
    bool    ClassName::Error_You_forgot_to_add_the_macro_CAF_PDM_XML_HEADER_INIT_and_or_CAF_PDM_XML_SOURCE_INIT_to_your_cpp_file_for_this_class() { return false;} \
    \
    QString ClassName::classKeyword() const \
    { \
        return classKeywordStatic(); \
    } \
    QString ClassName::classKeywordStatic() \
    { \
        return classKeywordAliases().front(); \
    } \
    std::vector<QString> ClassName::classKeywordAliases() \
    { \
        CAF_PDM_VERIFY_XML_KEYWORD(keyword) \
        return {keyword, ##__VA_ARGS__}; \
    } \
    bool ClassName::matchesClassKeyword(const QString& matchKeyword) const\
    { \
        auto aliases = classKeywordAliases(); \
        for (auto alias : aliases) \
        { \
            if (alias == matchKeyword) return true; \
        } \
        return false; \
    } \

/// CAF_PDM_XML_SOURCE_INIT associates the file keyword used for storage with the class and 
//  initializes the factory
/// Place this in the cpp file, preferably above the constructor
#define CAF_PDM_XML_SOURCE_INIT(ClassName, keyword, ...) \
    CAF_PDM_XML_ABSTRACT_SOURCE_INIT(ClassName, keyword, ##__VA_ARGS__) \
    static bool PDM_OBJECT_STRING_CONCATENATE(my##ClassName, __LINE__) = caf::PdmDefaultObjectFactory::instance()->registerCreator<ClassName>() 

#define CAF_PDM_XML_InitField(field, keyword) \
{ \
    CAF_PDM_VERIFY_XML_KEYWORD(keyword) \
    static bool chekingThePresenceOfHeaderAndSourceInitMacros =  \
            Error_You_forgot_to_add_the_macro_CAF_PDM_XML_HEADER_INIT_and_or_CAF_PDM_XML_SOURCE_INIT_to_your_cpp_file_for_this_class(); \
    this->isInheritedFromPdmXmlSerializable(); \
    \
    AddXmlCapabilityToField((field)); \
    addField((field), (keyword)); \
}
