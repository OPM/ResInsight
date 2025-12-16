#include "ValidationTest.h"

#include <QDebug>

CAF_PDM_SOURCE_INIT( ValidationTestObject, "ValidationTestObject" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ValidationTestObject::ValidationTestObject()
{
    CAF_PDM_InitObject( "Validation Test", "", "", "" );

    // Temperature field: -273.15 to 1000.0 Celsius
    CAF_PDM_InitField( &m_temperature, "temperature", 20.0, "Temperature (°C)", "", "", "" );
    m_temperature.uiCapability()->setUiToolTip( "Valid range: -273.15 to 1000.0 °C" );
    m_temperature.setRange( -273.15, 1000.0 );

    // Age field: 0 to 150 years
    CAF_PDM_InitField( &m_age, "age", 25, "Age (years)", "", "", "" );
    m_age.uiCapability()->setUiToolTip( "Valid range: 0 to 150 years" );
    m_age.setRange( 0, 150 );

    // Percentage field: 0 to 100
    CAF_PDM_InitField( &m_percentage, "percentage", 50.0, "Percentage (%)", "", "", "" );
    m_percentage.uiCapability()->setUiToolTip( "Valid range: 0 to 100%" );
    m_percentage.setRange( 0.0, 100.0 );

    // Count field: minimum value only (>= 0)
    CAF_PDM_InitField( &m_count, "count", 0, "Count", "", "", "" );
    m_count.uiCapability()->setUiToolTip( "Must be non-negative" );
    m_count.setMinValue( 0 );

    // Name field: no validation
    CAF_PDM_InitField( &m_name, "name", QString( "John Doe" ), "Name", "", "", "" );
    m_name.uiCapability()->setUiToolTip( "Free text field" );

    // Email field: no validation (but could add custom validation in validate())
    CAF_PDM_InitField( &m_email, "email", QString( "john@example.com" ), "Email", "", "", "" );
    m_email.uiCapability()->setUiToolTip( "Email address" );

    // Range field: -100.0 to 100.0
    CAF_PDM_InitField( &m_rangeField, "rangeField", 0.0, "Range Field", "", "", "" );
    m_rangeField.uiCapability()->setUiToolTip( "Valid range: -100.0 to 100.0" );
    m_rangeField.setRange( -100.0, 100.0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::map<QString, QString> ValidationTestObject::validate( const QString& configName ) const
{
    // First validate all fields using base implementation
    auto errors = PdmObject::validate( configName );

    // Add custom object-level validation

    // Email validation (basic)
    if ( !m_email().isEmpty() )
    {
        if ( !m_email().contains( "@" ) )
        {
            errors["email"] = "Email must contain @ symbol";
        }
    }

    // Name validation
    if ( m_name().isEmpty() )
    {
        errors["name"] = "Name cannot be empty";
    }

    // Cross-field validation: young people shouldn't have high percentages
    if ( m_age() < 18 && m_percentage() > 50.0 )
    {
        errors["_crossField"] = "For age < 18, percentage should not exceed 50%";
    }

    // Config-specific validation
    if ( configName == "strict" )
    {
        // In strict mode, require temperature to be in a narrower range
        if ( m_temperature() < 0.0 || m_temperature() > 100.0 )
        {
            errors["temperature"] = "In strict mode, temperature must be between 0 and 100 °C";
        }
    }

    return errors;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void ValidationTestObject::performValidation()
{
    qDebug() << "=== Validation Test ===";

    // Default validation
    auto errors = validate();

    if ( errors.empty() )
    {
        qDebug() << "All fields are valid!";
    }
    else
    {
        qDebug() << "Validation errors found:";
        for ( const auto& [field, error] : errors )
        {
            qDebug() << "  " << field << ":" << error;
        }
    }

    // Strict mode validation
    qDebug() << "\n=== Strict Mode Validation ===";
    auto strictErrors = validate( "strict" );

    if ( strictErrors.empty() )
    {
        qDebug() << "All fields are valid in strict mode!";
    }
    else
    {
        qDebug() << "Strict mode validation errors:";
        for ( const auto& [field, error] : strictErrors )
        {
            qDebug() << "  " << field << ":" << error;
        }
    }

    // Check individual fields
    qDebug() << "\n=== Individual Field Validation ===";
    qDebug() << "Temperature valid:" << m_temperature.isValid();
    qDebug() << "Age valid:" << m_age.isValid();
    qDebug() << "Percentage valid:" << m_percentage.isValid();
    qDebug() << "Count valid:" << m_count.isValid();

    // Show validation messages
    if ( !m_temperature.isValid() )
    {
        qDebug() << "  Temperature error:" << m_temperature.validate();
    }
    if ( !m_age.isValid() )
    {
        qDebug() << "  Age error:" << m_age.validate();
    }
    if ( !m_percentage.isValid() )
    {
        qDebug() << "  Percentage error:" << m_percentage.validate();
    }
    if ( !m_count.isValid() )
    {
        qDebug() << "  Count error:" << m_count.validate();
    }
}
