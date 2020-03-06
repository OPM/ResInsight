//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#ifndef CAF_IS_DEFINING_PDM_FIELD
#pragma once
#endif

#include "cafPdmValueField.h"

#include "cafAssert.h"
#include "cafInternalPdmValueFieldSpecializations.h"
#include "cafPdmUiFieldHandleInterface.h"

#include <QVariant>

#include <typeinfo>
#include <vector>


namespace caf 
{

class PdmObjectHandle;

//==================================================================================================
/// Field class encapsulating data with input and output of this data to/from a QXmlStream
/// read/write-FieldData is supposed to be specialized for types needing specialization
//==================================================================================================

template<typename DataType >
class PdmDataValueField : public PdmValueField
{
public:
    // Type traits magic to check if a template argument is a vector
    template<typename T> struct is_vector : public std::false_type {};
    template<typename T, typename A> struct is_vector<std::vector<T, A>> : public std::true_type {};

    typedef DataType FieldDataType;
    PdmDataValueField() {}
    PdmDataValueField(const PdmDataValueField& other)                   {  m_fieldValue = other.m_fieldValue; }
    explicit PdmDataValueField(const DataType& fieldValue)              {  m_fieldValue = fieldValue; }
    ~PdmDataValueField() override {}

    // Assignment 

    PdmDataValueField&  operator= (const PdmDataValueField& other)      {  CAF_ASSERT(isInitializedByInitFieldMacro()); m_fieldValue = other.m_fieldValue; return *this; }
    PdmDataValueField&  operator= (const DataType& fieldValue)          {  CAF_ASSERT(isInitializedByInitFieldMacro()); m_fieldValue = fieldValue; return *this; }

    // Basic access 

    DataType            value() const                                   { return m_fieldValue;  }
    void                setValue(const DataType& fieldValue)            { CAF_ASSERT(isInitializedByInitFieldMacro()); m_fieldValue = fieldValue; }
    void                setValueWithFieldChanged(const DataType& fieldValue);

    // Implementation of PdmValueField interface

    QVariant    toQVariant() const override                              { CAF_ASSERT(isInitializedByInitFieldMacro()); return PdmValueFieldSpecialization<DataType>::convert(m_fieldValue); }
    void        setFromQVariant(const QVariant& variant) override        { CAF_ASSERT(isInitializedByInitFieldMacro()); PdmValueFieldSpecialization<DataType>::setFromVariant(variant, m_fieldValue);   }
    bool        isReadOnly() const override                              { return false;  }

    // Access operators

    /*Conversion */     operator DataType () const                      { return m_fieldValue; }
    const DataType&     operator()() const                              { return m_fieldValue; }

    DataType&           v()                                             { return m_fieldValue; } // This one breaches encapsulation. Remove ?
    const DataType&     v() const                                       { return m_fieldValue; }

    bool                operator== (const DataType& fieldValue) const   { return m_fieldValue == fieldValue; }
    bool                operator!= (const DataType& fieldValue) const   { return m_fieldValue != fieldValue; }

protected:
    DataType            m_fieldValue;

    // Default value stuff. 
    //
    // This is not used enough. Remove when dust has settled. 
    // ResInsight uses at one point to find whether the value is changed by the user
public:
    const DataType&     defaultValue() const                            { return m_defaultFieldValue; }
    void                setDefaultValue(const DataType& val)            { m_defaultFieldValue = val; }
protected:
    DataType            m_defaultFieldValue;

};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void caf::PdmDataValueField<DataType>::setValueWithFieldChanged(const DataType& fieldValue)
{
    CAF_ASSERT(isInitializedByInitFieldMacro());

    PdmUiFieldHandleInterface* uiFieldHandleInterface = capability<PdmUiFieldHandleInterface>();
    if (uiFieldHandleInterface)
    {
        QVariant oldValue = uiFieldHandleInterface->toUiBasedQVariant();

        m_fieldValue = fieldValue;

        QVariant newUiBasedQVariant = uiFieldHandleInterface->toUiBasedQVariant();

        uiFieldHandleInterface->notifyFieldChanged(oldValue, newUiBasedQVariant);
    }
    else
    {
        m_fieldValue = fieldValue;
    }
}

} // End of namespace caf
