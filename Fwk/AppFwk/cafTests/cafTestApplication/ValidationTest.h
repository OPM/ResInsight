#pragma once

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

/// Test object demonstrating field validation features
class ValidationTestObject : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    ValidationTestObject();

    // Override validate to add custom object-level validation
    std::map<QString, QString> validate( const QString& configName = "" ) const override;

    // Validation check method
    void performValidation();

public:
    // Fields with range validation
    caf::PdmField<double> m_temperature;
    caf::PdmField<int>    m_age;
    caf::PdmField<double> m_percentage;
    caf::PdmField<int>    m_count;

    // Fields without validation
    caf::PdmField<QString> m_name;
    caf::PdmField<QString> m_email;

    // Field with both min and max validation
    caf::PdmField<double> m_rangeField;
};
