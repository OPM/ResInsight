#pragma once


namespace caf
{

//--------------------------------------------------------------------------------------------------
/// This method is supposed to be the interface for the implementation of UI editors to set values into 
/// the field. The data to set must be encapsulated in a QVariant. 
/// This method triggers PdmObject::fieldChangedByUi() and PdmObject::updateConnectedEditors(), an thus
/// makes the application and the UI aware of the change.
///
/// Note : If the field has optionValues the interface is _index-based_. The QVariant must contain 
///        an UInt representing the index to the option selected by the user interface.
/// 
//--------------------------------------------------------------------------------------------------

template<typename FieldType >
void caf::PdmFieldUiCap<FieldType>::setValueFromUi(const QVariant& uiValue)
{
    QVariant oldUiBasedQVariant = toUiBasedQVariant();

    // Check whether we are handling selections of values or actual values
    if (m_optionEntryCache.size())
    {
        // This has an option based GUI, the uiValue is only indexes into the m_optionEntryCache
        if (uiValue.type() == QVariant::UInt)
        {
            assert(uiValue.toUInt() < static_cast<unsigned int>(m_optionEntryCache.size()));
            typename FieldType::FieldDataType value;
            PdmUiFieldSpecialization<typename FieldType::FieldDataType>::setFromVariant(m_optionEntryCache[uiValue.toUInt()].value, value);
            m_field->setValue(value);
        }
        else if (uiValue.type() == QVariant::List)
        {
            QList<QVariant> selectedIndexes = uiValue.toList();
            QList<QVariant> valuesToSetInField;

            if (selectedIndexes.isEmpty())
            {
                typename FieldType::FieldDataType value;
                PdmUiFieldSpecialization<typename FieldType::FieldDataType>::setFromVariant(valuesToSetInField, value);
                m_field->setValue(value);
            }
            else
            {
                if (selectedIndexes.front().type() == QVariant::UInt)
                {
                    for (int i = 0; i < selectedIndexes.size(); ++i)
                    {
                        unsigned int opIdx = selectedIndexes[i].toUInt();
                        if (opIdx < static_cast<unsigned int>(m_optionEntryCache.size()))
                        {
                            valuesToSetInField.push_back(m_optionEntryCache[opIdx].value);
                        }
                    }
                    typename FieldType::FieldDataType value;
                    PdmUiFieldSpecialization<typename FieldType::FieldDataType>::setFromVariant(valuesToSetInField, value);
                    m_field->setValue(value);
                }
                else
                {
                    // We are not getting indexes as expected from the UI. For now assert, to catch this condition
                    // but it should possibly be handled as setting the values explicitly. The code for that is below the assert
                    assert(false);
                    typename FieldType::FieldDataType value;
                    PdmUiFieldSpecialization<typename FieldType::FieldDataType>::setFromVariant(uiValue, value);
                    m_field->setValue(value);
                    m_optionEntryCache.clear();
                }
            }

        }
        else
        {
            // We are not getting indexes as expected from the UI. For now assert, to catch this condition
            // but it should possibly be handled as setting the values explicitly. The code for that is below the assert
            assert(false);
            typename FieldType::FieldDataType value;
            PdmUiFieldSpecialization<typename FieldType::FieldDataType>::setFromVariant(uiValue, value);
            m_field->setValue(value);
            m_optionEntryCache.clear();
        }
    }
    else
    {   // Not an option based GUI, the uiValue is a real field value
        typename FieldType::FieldDataType value;
        PdmUiFieldSpecialization<typename FieldType::FieldDataType>::setFromVariant(uiValue, value);
        m_field->setValue(value);
    }

    QVariant newUiBasedQVariant = toUiBasedQVariant();

    this->notifyFieldChanged(oldUiBasedQVariant, newUiBasedQVariant);
}


//--------------------------------------------------------------------------------------------------
/// Extracts a QVariant representation of the data in the field to be used in the UI. 
/// 
/// Note : For fields with a none-empty valueOptions list, the returned QVariant contains the 
///        _indexes_ to the selected options rather than the actual values, if they can be found.
///
///        If this is a multivalue field, and we cant find all of the field values among the options, 
///        the method asserts (For now), forcing the valueOptions to always contain the field values.
///        Single value fields will return -1 if the option is not found, allowing the concept of "nothing selected"
//--------------------------------------------------------------------------------------------------

template<typename FieldType >
QVariant caf::PdmFieldUiCap<FieldType>::uiValue() const
{
    if (m_optionEntryCache.size())
    {
        QVariant uiBasedQVariant = toUiBasedQVariant();
        std::vector<unsigned int> indexesToFoundOptions;
        PdmOptionItemInfo::findValues<typename FieldType::FieldDataType>(m_optionEntryCache, uiBasedQVariant, indexesToFoundOptions);
        if (uiBasedQVariant.type() == QVariant::List)
        {
            if (indexesToFoundOptions.size() == static_cast<size_t>(uiBasedQVariant.toList().size()))
            {
                QList<QVariant> returnList;
                for (size_t i = 0; i < indexesToFoundOptions.size(); ++i)
                {
                    returnList.push_back(QVariant(indexesToFoundOptions[i]));
                }
                return QVariant(returnList);
            }
            assert(false); // Did not find all the field values among the options available.
        }
        else
        {
            if (indexesToFoundOptions.size() == 1) return QVariant(indexesToFoundOptions.front());
            else return QVariant(-1); // Return -1 if not found instead of assert. Should result in clearing the selection
        }

        assert(false);
        return uiBasedQVariant;
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

template<typename FieldType >
QList<PdmOptionItemInfo> caf::PdmFieldUiCap<FieldType>::valueOptions(bool* useOptionsOnly)
{
    // First check if the owner PdmObject has a value options specification. 
    // if it has, use it.
    if (m_field->ownerObject())
    {
        m_optionEntryCache = uiObj(m_field->ownerObject())->calculateValueOptions(this->m_field, useOptionsOnly);
        if (m_optionEntryCache.size())
        {
            // Make sure the options contain the field values, event though they not necessarily 
            // is supplied as possible options by the application. This is a convenience making sure
            // the actual data in the pdmObject is shown correctly in the UI, and to prevent accidental 
            // changes of the field values

            // Find the field value(s) in the list if present

            QVariant uiBasedQVariant = toUiBasedQVariant();

            std::vector<unsigned int> foundIndexes;
            bool foundAllFieldValues = PdmOptionItemInfo::findValues<typename FieldType::FieldDataType>(m_optionEntryCache, uiBasedQVariant, foundIndexes);

            // If not all are found, we have to add the missing to the list, to be able to show it

            if (isAutoAddingOptionFromValue() && !foundAllFieldValues)
            {
                if (uiBasedQVariant.type() != QVariant::List)  // Single value field
                {
                    if (!uiBasedQVariant.toString().isEmpty())
                    {
                        m_optionEntryCache.push_front(PdmOptionItemInfo(uiBasedQVariant.toString(), uiBasedQVariant, true, QIcon()));
                    }
                }
                else // The field value is a list of values 
                {
                    QList<QVariant> valuesSelectedInField = uiBasedQVariant.toList();
                    for (int i= 0 ; i < valuesSelectedInField.size(); ++i)
                    {
                        bool isFound = false;
                        for (unsigned int opIdx = 0; opIdx < static_cast<unsigned int>(m_optionEntryCache.size()); ++opIdx)
                        {
                            if (valuesSelectedInField[i] == m_optionEntryCache[opIdx].value) isFound = true;
                        }

                        if (!isFound && !valuesSelectedInField[i].toString().isEmpty())
                        {
                            m_optionEntryCache.push_front(PdmOptionItemInfo(valuesSelectedInField[i].toString(), valuesSelectedInField[i], true, QIcon()));
                        }
                    }
                }
            }

            return m_optionEntryCache;
        }
    }

    // If we have no options, use the options defined by the type. Normally only caf::AppEnum type

#if 0
    m_optionEntryCache = PdmFieldTypeSpecialization<typename FieldType::FieldDataType>::valueOptions(useOptionsOnly, m_fieldValue);
    return m_optionEntryCache;
#else
    return PdmUiFieldSpecialization<typename FieldType::FieldDataType>::valueOptions(useOptionsOnly, m_field->value());
#endif

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename FieldType>
QVariant caf::PdmFieldUiCap<FieldType>::toUiBasedQVariant() const
{
    return PdmUiFieldSpecialization<typename FieldType::FieldDataType>::convert(m_field->value());
}


} // End of namespace caf

