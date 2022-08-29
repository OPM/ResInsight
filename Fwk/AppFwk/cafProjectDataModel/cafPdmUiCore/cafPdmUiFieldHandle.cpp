#include "cafPdmUiFieldHandle.h"

#include "cafAssert.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmUiEditorHandle.h"
#include "cafPdmUiModelChangeDetector.h"
#include "cafPdmUiObjectHandle.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiFieldHandle::PdmUiFieldHandle( PdmFieldHandle* owner, bool giveOwnership )
    : m_isAutoAddingOptionFromValue( true )
    , m_useAutoValue( false )
    , m_isAutoValueSupported( false )
{
    m_owner = owner;
    owner->addCapability( this, giveOwnership );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUiFieldHandle::~PdmUiFieldHandle()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* PdmUiFieldHandle::fieldHandle()
{
    return m_owner;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant PdmUiFieldHandle::uiValue() const
{
    return QVariant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> PdmUiFieldHandle::valueOptions() const
{
    return QList<PdmOptionItemInfo>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiFieldHandle::notifyFieldChanged( const QVariant& oldFieldValue, const QVariant& newFieldValue )
{
    if ( !this->isQVariantDataEqual( oldFieldValue, newFieldValue ) )
    {
        PdmFieldHandle* fieldHandle = this->fieldHandle();
        CAF_ASSERT( fieldHandle && fieldHandle->ownerObject() );

        PdmObjectHandle* ownerObjectHandle = fieldHandle->ownerObject();

        {
            bool noOwnerObject = true;

            // Object editors

            PdmUiObjectHandle* uiObjHandle = uiObj( ownerObjectHandle );
            if ( uiObjHandle )
            {
                uiObjHandle->fieldChangedByUi( fieldHandle, oldFieldValue, newFieldValue );
                uiObjHandle->updateConnectedEditors();

                noOwnerObject = false;
            }

            // Field editors

            for ( const auto& editorForThisField : m_editors )
            {
                PdmUiEditorHandle* editorContainingThisField = editorForThisField->topMostContainingEditor();

                bool editorContainingThisFieldIsNotUpdated = !uiObjHandle->hasEditor( editorContainingThisField );

                if ( noOwnerObject || editorContainingThisFieldIsNotUpdated )
                {
                    editorContainingThisField->updateUi();
                }
            }
        }

        if ( ownerObjectHandle->parentField() && ownerObjectHandle->parentField()->ownerObject() )
        {
            PdmUiObjectHandle* uiObjHandle = uiObj( ownerObjectHandle->parentField()->ownerObject() );
            if ( uiObjHandle )
            {
                uiObjHandle->childFieldChangedByUi( ownerObjectHandle->parentField() );

                // If updateConnectedEditors() is required, this has to be called in childFieldChangedByUi()
            }
        }

        PdmUiModelChangeDetector::instance()->setModelChanged();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiFieldHandle::isAutoAddingOptionFromValue() const
{
    return m_isAutoAddingOptionFromValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiFieldHandle::setAutoAddingOptionFromValue( bool isAddingValue )
{
    m_isAutoAddingOptionFromValue = isAddingValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiFieldHandle::enableAndSetAutoValue( const QVariant& autoValue )
{
    setAutoValue( autoValue );
    enableAutoValue( true );
    m_isAutoValueSupported = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiFieldHandle::setAutoValue( const QVariant& autoValue )
{
    m_autoValue = autoValue;

    if ( m_useAutoValue )
    {
        setValueFromUiEditor( m_autoValue, false );
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QVariant PdmUiFieldHandle::autoValue() const
{
    return m_autoValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiFieldHandle::enableAutoValue( bool enable )
{
    m_useAutoValue = enable;

    if ( m_useAutoValue )
    {
        setValueFromUiEditor( m_autoValue, false );
        updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiFieldHandle::isAutoValueEnabled() const
{
    return m_useAutoValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiFieldHandle::enableAutoValueSupport( bool enable )
{
    m_isAutoValueSupported = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiFieldHandle::isAutoValueSupported() const
{
    return m_isAutoValueSupported;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString>> PdmUiFieldHandle::attributes() const
{
    std::vector<std::pair<QString, QString>> attr;

    if ( m_useAutoValue )
    {
        attr.push_back( { "autoValueEnabled", "true" } );
    }

    if ( m_isAutoValueSupported )
    {
        attr.push_back( { "autoValueSupported", "true" } );
    }

    return attr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiFieldHandle::setAttributes( const std::vector<std::pair<QString, QString>>& attributes )
{
    for ( auto [key, valueString] : attributes )
    {
        valueString = valueString.toUpper();

        if ( key == "autoValueEnabled" )
        {
            if ( valueString == "TRUE" )
            {
                enableAutoValue( true );
            }
        }
        else if ( key == "autoValueSupported" )
        {
            if ( valueString == "TRUE" )
            {
                enableAutoValueSupport( true );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiFieldHandle::setValueFromUiEditor( const QVariant& uiValue, bool notifyFieldChanged )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool PdmUiFieldHandle::isQVariantDataEqual( const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant ) const
{
    CAF_ASSERT( false );
    return false;
}

//--------------------------------------------------------------------------------------------------
/// Implementation of uiCapability() defined in cafPdmFieldHandle.h
//--------------------------------------------------------------------------------------------------
PdmUiFieldHandle* PdmFieldHandle::uiCapability()
{
    PdmUiFieldHandle* uiField = capability<PdmUiFieldHandle>();
    CAF_ASSERT( uiField );

    return uiField;
}

} // End of namespace caf
