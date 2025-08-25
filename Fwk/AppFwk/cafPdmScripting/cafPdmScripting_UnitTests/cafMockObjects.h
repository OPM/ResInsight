
#pragma once

#include "gtest/gtest.h"

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmCodeGenerator.h"
#include "cafPdmDocument.h"
#include "cafPdmField.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectGroup.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmPointer.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPythonGenerator.h"
#include "cafPdmReferenceHelper.h"

#include <QFile>
#include <memory>

/// Demo objects to show the usage of the Pdm system
enum class MyEnum
{
    T1,
    T2,
    T3,
    T4,
    T5,
    T6
};

// This is a specialized class that can be used to implement custom behavior in configureCapabilities
// It is required to have a specialization of PdmFieldScriptingCapability for this class, as the new class
// MyAppEnumField is not known to the template
class MyAppEnumField : public caf::PdmField<caf::AppEnum<MyEnum>>
{
public:
    MyAppEnumField() {

    };

    void configureCapabilities() override {};
};

namespace caf
{
template <>
class PdmFieldScriptingCapability<MyAppEnumField> : public PdmFieldScriptingCapability<caf::PdmField<caf::AppEnum<MyEnum>>>
{
public:
    PdmFieldScriptingCapability( MyAppEnumField* field, const QString& fieldName, bool giveOwnership )
        : PdmFieldScriptingCapability<caf::PdmField<caf::AppEnum<MyEnum>>>( field, fieldName, giveOwnership )
    {
    }

    QString dataType() const override { return "str"; }
};
}; // namespace caf

class SimpleObj : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    SimpleObj();

    /// Assignment and copying of PDM objects is not focus for the features. This is only a
    /// "would it work" test
    SimpleObj( const SimpleObj& other );

    ~SimpleObj() {}

    caf::PdmField<double>              m_position;
    caf::PdmField<double>              m_dir;
    caf::PdmField<double>              m_up;
    caf::PdmField<std::vector<double>> m_numbers;
    caf::PdmProxyValueField<double>    m_proxyDouble;

    void   setDoubleMember( const double& d );
    double doubleMember() const;

    double m_doubleMember;
};

class DemoPdmObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    DemoPdmObject();

    ~DemoPdmObject();

    // Fields
    caf::PdmField<double>  m_doubleField;
    caf::PdmField<int>     m_intField;
    caf::PdmField<QString> m_textField;
    MyAppEnumField         m_myAppEnum;

    caf::PdmChildField<SimpleObj*> m_simpleObjPtrField;
    caf::PdmChildField<SimpleObj*> m_simpleObjPtrField2;
};

class InheritedDemoObj : public DemoPdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum TestEnumType
    {
        T1,
        T2,
        T3
    };

    InheritedDemoObj();

    caf::PdmField<std::vector<QString>>  m_texts;
    caf::PdmField<std::vector<double>>   m_numbers;
    caf::PdmField<std::optional<double>> m_optionalNumber;

    caf::PdmField<caf::AppEnum<TestEnumType>> m_testEnumField;
    caf::PdmChildArrayField<SimpleObj*>       m_simpleObjectsField;
};

class MyPdmDocument : public caf::PdmDocument
{
    CAF_PDM_HEADER_INIT;

public:
    MyPdmDocument();

    caf::PdmChildArrayField<PdmObjectHandle*> objects;
};
