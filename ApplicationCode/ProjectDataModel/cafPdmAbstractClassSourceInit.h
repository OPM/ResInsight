#pragma once


#define CAF_PDM_ABSTRACT_SOURCE_INIT(ClassName, keyword) \
    bool    ClassName::Error_You_forgot_to_add_the_macro_CAF_PDM_XML_HEADER_INIT_and_or_CAF_PDM_XML_SOURCE_INIT_to_your_cpp_file_for_this_class() { return false; } \
    QString ClassName::classKeywordStatic() { assert(PdmXmlObjectHandle::isValidXmlElementName(keyword)); return keyword; }
