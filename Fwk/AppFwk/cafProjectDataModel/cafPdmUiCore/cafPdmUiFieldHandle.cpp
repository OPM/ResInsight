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

        // The user has edited this field, diable use of auto value
        enableAutoValue( false );

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
    if ( m_useAutoValue ) setValueFromUiEditor( m_autoValue, true );
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
bool PdmUiFieldHandle::isAutoValueSupported() const
{
    return m_isAutoValueSupported;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString>> PdmUiFieldHandle::attributes() const
{
    if ( m_useAutoValue )
    {
        return { { "autoValue", "true" } };
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmUiFieldHandle::setAttributes( const std::vector<std::pair<QString, QString>>& attributes )
{
    for ( auto [key, value] : attributes )
    {
        if ( key == "autoValue" )
        {
            if ( value.toUpper() == "TRUE" )
            {
                enableAutoValue( true );
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
