#pragma once

namespace caf
{
//--------------------------------------------------------------------------------------------------
/// This method is supposed to be the interface for the implementation of UI editors to set values into
/// the field. The data to set must be encapsulated in a QVariant.
/// This method triggers PdmObject::fieldChangedByUi() and PdmObject::updateConnectedEditors(), an thus
/// makes the application and the UI aware of the change.
///
/// Note : If the field has m_optionEntryCache the interface is _index-based_. The QVariant must contain
///        an UInt representing the index to the option selected by the user interface.
///
//--------------------------------------------------------------------------------------------------

template <typename FieldType>
void PdmFieldUiCap<FieldType>::setValueFromUiEditor( const QVariant& uiValue, bool notifyFieldChanged )
{
    QVariant oldUiBasedQVariant = toUiBasedQVariant();

    bool setUiValueDirectly = false;

    // Check whether we are handling selections of values or actual values
    if ( !m_optionEntryCache.empty() )
    {
        // This has an option based GUI, the uiValue is only indexes into the m_optionEntryCache
        if ( uiValue.metaType().id() == QMetaType::UInt )
        {
            uint optionIndex = uiValue.toUInt();
            CAF_ASSERT( optionIndex < static_cast<unsigned int>( m_optionEntryCache.size() ) );

            QVariant optionVariantValue = m_optionEntryCache[optionIndex].value();

            typename FieldType::FieldDataType fieldValue;
            PdmUiFieldSpecialization<typename FieldType::FieldDataType>::setFromVariant( optionVariantValue, fieldValue );
            m_field->setValue( fieldValue );
        }
        else if ( uiValue.metaType().id() == QMetaType::QVariantList )
        {
            QList<QVariant> selectedIndexes = uiValue.toList();
            QList<QVariant> valuesToSetInField;

            if ( selectedIndexes.isEmpty() )
            {
                typename FieldType::FieldDataType value;
                PdmUiFieldSpecialization<typename FieldType::FieldDataType>::setFromVariant( valuesToSetInField, value );
                m_field->setValue( value );
            }
            else
            {
                if ( selectedIndexes.front().metaType().id() == QMetaType::UInt )
                {
                    for ( int i = 0; i < selectedIndexes.size(); ++i )
                    {
                        unsigned int opIdx = selectedIndexes[i].toUInt();
                        if ( opIdx < static_cast<unsigned int>( m_optionEntryCache.size() ) )
                        {
                            valuesToSetInField.push_back( m_optionEntryCache[opIdx].value() );
                        }
                    }
                    typename FieldType::FieldDataType value;
                    PdmUiFieldSpecialization<typename FieldType::FieldDataType>::setFromVariant( valuesToSetInField, value );
                    m_field->setValue( value );
                }
                else
                {
                    // We are not getting indexes as expected from the UI. For now assert, to catch this condition
                    // but it should possibly be handled as setting the values explicitly. The code for that is below
                    // the assert
                    CAF_ASSERT( false );
                    typename FieldType::FieldDataType value;
                    PdmUiFieldSpecialization<typename FieldType::FieldDataType>::setFromVariant( uiValue, value );
                    m_field->setValue( value );
                    m_optionEntryCache.clear();
                }
            }
        }
        else
        {
            // We are not getting indexes as usually expected when an option cache is present.
            // This situation can occur if a text field is edited by a combobox allowing user defined input
            // when a history of recently used strings are stored in a field of string

            setUiValueDirectly = true;
        }
    }
    else
    {
        // Not an option based GUI, the uiValue is a real field value

        setUiValueDirectly = true;
    }

    if ( setUiValueDirectly )
    {
        typename FieldType::FieldDataType value;
        PdmUiFieldSpecialization<typename FieldType::FieldDataType>::setFromVariant( uiValue, value );
        m_field->setValue( value );
    }

    QVariant newUiBasedQVariant = toUiBasedQVariant();

    if ( notifyFieldChanged ) this->notifyFieldChanged( oldUiBasedQVariant, newUiBasedQVariant );
}

//--------------------------------------------------------------------------------------------------
/// Extracts a QVariant representation of the data in the field to be used in the UI.
///
/// Note : You have to call valueOptions() before this method to make sure that the m_optionEntryCache is updated and
/// valid !!!
///        We should find a way to enforce this, but JJS and MSJ could not think of a way to catch the situation
///        that would always be valid. Could invalidate cache when all editors are removed. That would help.
///        It is not considered to be healthy to always call valueOptions() from this method either -> double calls to
///        valueOptions() The solution might actually be to merge the two ino one method, making uiValues and
///        valueOptions directly connected.
///
/// Note : For fields with a none-empty m_optionEntryCache list, the returned QVariant contains the
///        _indexes_ to the selected options rather than the actual values, if they can be found.
///
///        If this is a multivalue field, and we can't find all of the field values among the options,
///        the method asserts (For now), forcing the valueOptions to always contain the field values.
///        Single value fields will return -1 if the option is not found, allowing the concept of "nothing selected"
//--------------------------------------------------------------------------------------------------

template <typename FieldType>
QVariant PdmFieldUiCap<FieldType>::uiValue() const
{
    if ( !m_optionEntryCache.empty() )
    {
        QVariant                  uiBasedQVariant = toUiBasedQVariant();
        std::vector<unsigned int> indexesToFoundOptions;
        PdmOptionItemInfo::findValues<typename FieldType::FieldDataType>( m_optionEntryCache,
                                                                          uiBasedQVariant,
                                                                          indexesToFoundOptions );
        if ( uiBasedQVariant.metaType().id() == QMetaType::QVariantList )
        {
            if ( isAutoAddingOptionFromValue() &&
                 indexesToFoundOptions.size() != static_cast<size_t>( uiBasedQVariant.toList().size() ) )
            {
                return QVariant();

                // CAF_ASSERT( false ); // Did not find all the field values among the options available, even though we
                // should. Reasons might be:
                //    The "core" data type in the field is probably not supported by
                //    QVariant::toString() You forgot to call valueOptions() before the call to
                //    uiValue().
            }
            else
            {
                QList<QVariant> returnList;
                for ( size_t i = 0; i < indexesToFoundOptions.size(); ++i )
                {
                    returnList.push_back( QVariant( indexesToFoundOptions[i] ) );
                }
                return QVariant( returnList );
            }
        }

        if ( indexesToFoundOptions.size() == 1 ) return QVariant( indexesToFoundOptions.front() );

        // Return -1 if not found, expected result in clearing the selection
        return QVariant( -1 );
    }
    else
    {
        return toUiBasedQVariant();
    }
}

//--------------------------------------------------------------------------------------------------
/// Returns the option values that is to be displayed in the UI for this field.
/// This method calls the virtual PdmObject::calculateValueOptions to get the list provided from the
/// application, then possibly adds the current field value(s) to the list, to
/// make sure the actual values are shown
///
/// Note: This method is missing the uiConfigName concept. This is a Todo. The m_optionEntryCache
///         then needs to be stored pr. uiConfigName.
//--------------------------------------------------------------------------------------------------

template <typename FieldType>
QList<PdmOptionItemInfo> PdmFieldUiCap<FieldType>::valueOptions() const
{
    m_optionEntryCache.clear();

    // First check if the owner PdmObject has a value options specification. If it has, we use it.
    if ( m_field->ownerObject() )
    {
        m_optionEntryCache = uiObj( m_field->ownerObject() )->calculateValueOptions( this->m_field );
    }

    // Check if the field can provide value option
    if ( m_optionEntryCache.empty() && m_valueOptionsGenerator )
    {
        m_optionEntryCache = m_valueOptionsGenerator();
    }

    // If we got no options, use the options defined by the type. Normally only caf::AppEnum type

    if ( m_optionEntryCache.empty() )
    {
        auto keyword = m_field->keyword();
        m_optionEntryCache =
            PdmUiFieldSpecialization<typename FieldType::FieldDataType>::valueOptions( keyword, m_field->value() );
    }

    if ( !m_optionEntryCache.empty() && isAutoAddingOptionFromValue() )
    {
        // Make sure the options contain the field values, event though they not necessarily
        // is supplied as possible options by the application. This is a convenience making sure
        // the actual data in the pdmObject is shown correctly in the UI, and to prevent accidental
        // changes of the field values

        // Find the field value(s) in the list if present

        QVariant uiBasedQVariant = toUiBasedQVariant();

        std::vector<unsigned int> foundIndexes;
        bool foundAllFieldValues = PdmOptionItemInfo::findValues<typename FieldType::FieldDataType>( m_optionEntryCache,
                                                                                                     uiBasedQVariant,
                                                                                                     foundIndexes );

        // If not all are found, we have to add the missing to the list, to be able to show it
        // This will only work if the field data type (or element type for containers) is supported by
        // QVariant.toString(). Custom classes don't

        if ( !foundAllFieldValues )
        {
            if ( uiBasedQVariant.metaType().id() != QMetaType::QVariantList ) // Single value field
            {
                if ( !uiBasedQVariant.toString().isEmpty() )
                {
                    m_optionEntryCache.push_front( PdmOptionItemInfo( uiBasedQVariant.toString(), uiBasedQVariant, true ) );
                }
            }
            else // The field value is a list of values
            {
                QList<QVariant> valuesSelectedInField = uiBasedQVariant.toList();
                for ( int i = 0; i < valuesSelectedInField.size(); ++i )
                {
                    bool isFound = false;
                    for ( unsigned int opIdx = 0; opIdx < static_cast<unsigned int>( m_optionEntryCache.size() ); ++opIdx )
                    {
                        if ( PdmUiFieldSpecialization<
                                 typename FieldType::FieldDataType>::isDataElementEqual( valuesSelectedInField[i],
                                                                                         m_optionEntryCache[opIdx].value() ) )
                        {
                            isFound = true;
                        }
                    }

                    if ( !isFound && !valuesSelectedInField[i].toString().isEmpty() )
                    {
                        m_optionEntryCache.push_front(
                            PdmOptionItemInfo( valuesSelectedInField[i].toString(), valuesSelectedInField[i], true ) );
                    }
                }
            }
        }
    }

    return m_optionEntryCache;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
QVariant PdmFieldUiCap<FieldType>::toUiBasedQVariant() const
{
    return PdmUiFieldSpecialization<typename FieldType::FieldDataType>::convert( m_field->value() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
bool PdmFieldUiCap<FieldType>::isQVariantDataEqual( const QVariant& oldUiBasedQVariant,
                                                    const QVariant& newUiBasedQVariant ) const
{
    return PdmValueFieldSpecialization<typename FieldType::FieldDataType>::isEqual( oldUiBasedQVariant, newUiBasedQVariant );
}

} // End of namespace caf
