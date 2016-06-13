#pragma once

#include "cafPdmDefaultObjectFactory.h"

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


/// CAF_PDM_HEADER_INIT assists the factory used when reading objects from file
/// Place this in the header file inside the class definition of your PdmObject

// To be renamed CAF_PDM_XML_HEADER_INIT
#define CAF_PDM_XML_HEADER_INIT \
public: \
    virtual QString classKeyword()   { return  classKeywordStatic(); } \
    static  QString classKeywordStatic(); \
    \
    static  bool Error_You_forgot_to_add_the_macro_CAF_PDM_XML_HEADER_INIT_and_or_CAF_PDM_XML_SOURCE_INIT_to_your_cpp_file_for_this_class()


/// CAF_PDM_XML_SOURCE_INIT associates the file keyword used for storage with the class and 
//  initializes the factory
/// Place this in the cpp file, preferably above the constructor

#define CAF_PDM_XML_SOURCE_INIT(ClassName, keyword) \
    bool    ClassName::Error_You_forgot_to_add_the_macro_CAF_PDM_XML_HEADER_INIT_and_or_CAF_PDM_XML_SOURCE_INIT_to_your_cpp_file_for_this_class() { return false;} \
    \
    QString ClassName::classKeywordStatic() { return keyword;   } \
    static bool PDM_OBJECT_STRING_CONCATENATE(my##ClassName, __LINE__) = caf::PdmDefaultObjectFactory::instance()->registerCreator<ClassName>() 

#define CAF_PDM_XML_ABSTRACT_SOURCE_INIT(ClassName, keyword) \
    bool    ClassName::Error_You_forgot_to_add_the_macro_CAF_PDM_XML_HEADER_INIT_and_or_CAF_PDM_XML_SOURCE_INIT_to_your_cpp_file_for_this_class() { return false;} \
    \
    QString ClassName::classKeywordStatic() { return keyword;   } \

#define CAF_PDM_XML_InitField(field, keyword) \
{ \
    static bool chekingThePresenceOfHeaderAndSourceInitMacros =  \
            Error_You_forgot_to_add_the_macro_CAF_PDM_XML_HEADER_INIT_and_or_CAF_PDM_XML_SOURCE_INIT_to_your_cpp_file_for_this_class(); \
    this->isInheritedFromPdmXmlSerializable(); \
    \
    AddXmlCapabilityToField((field)); \
    addField((field), (keyword)); \
}
