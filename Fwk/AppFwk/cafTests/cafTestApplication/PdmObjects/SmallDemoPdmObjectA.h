#pragma once

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmPtrField.h"

#include <QString>

class SmallDemoPdmObjectA : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class TestEnumType
    {
        T1 = 10,
        T2,
        T3
    };

    SmallDemoPdmObjectA();

    caf::PdmFieldHandle* objectToggleField() override;
    caf::PdmFieldHandle* userDescriptionField() override;

    void migrateFieldContent( QString& fieldContent, caf::PdmFieldHandle* fieldHandle ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

    void                       setEnumMember( const caf::AppEnum<TestEnumType>& val );
    caf::AppEnum<TestEnumType> enumMember() const;

    void enableAutoValueForTestEnum( TestEnumType value );
    void enableAutoValueForDouble( double value );
    void enableAutoValueForInt( double value );

    void setAutoValueForTestEnum( TestEnumType value );
    void setAutoValueForDouble( double value );
    void setAutoValueForInt( double value );

protected:
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

    void defineObjectEditorAttribute( QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;

private:
    caf::PdmField<double>                     m_doubleField;
    caf::PdmField<int>                        m_intField;
    caf::PdmField<QString>                    m_textField;
    caf::PdmField<caf::AppEnum<TestEnumType>> m_testEnumField;
    caf::PdmPtrField<SmallDemoPdmObjectA*>    m_ptrField;

    caf::PdmProxyValueField<caf::AppEnum<TestEnumType>> m_proxyEnumField;
    TestEnumType                                        m_proxyEnumMember;

    caf::PdmField<std::vector<caf::AppEnum<TestEnumType>>> m_multipleAppEnum;
    caf::PdmField<caf::AppEnum<TestEnumType>>              m_highlightedEnum;

    caf::PdmField<bool> m_toggleField;
    caf::PdmField<bool> m_pushButtonField;
};
