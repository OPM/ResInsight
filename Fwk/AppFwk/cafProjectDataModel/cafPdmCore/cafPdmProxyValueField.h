#pragma once

#include "cafAssert.h"
#include "cafInternalPdmValueFieldSpecializations.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmPointer.h"
#include "cafPdmValueField.h"

#include <QVariant>

#include <type_traits>
#include <vector>

namespace caf
{
//==================================================================================================
/// Abstract non-templated base class for PdmProxyValueField
/// Exists only to be able to determine that a field is a proxy field
//==================================================================================================

class PdmProxyFieldHandle : public PdmValueField
{
public:
    virtual bool isStreamingField() const = 0;
    virtual bool hasGetter() const        = 0;
    virtual bool hasSetter() const        = 0;
};

//==================================================================================================
/// Field class encapsulating data access through object setter/getter with input and output of this
/// data to/from a QXmlStream
/// read/write-FieldData is supposed to be specialized for types needing specialization
//==================================================================================================
template <typename DataType>
class PdmProxyValueField : public PdmProxyFieldHandle
{
public:
    // Type traits magic to check if a template argument is a vector
    template <typename T>
    struct is_vector : public std::false_type
    {
    };
    template <typename T, typename A>
    struct is_vector<std::vector<T, A>> : public std::true_type
    {
    };

    typedef DataType FieldDataType;
    PdmProxyValueField()
    {
        m_valueSetter = NULL;
        m_valueGetter = NULL;
    }
    ~PdmProxyValueField() override
    {
        if ( m_valueSetter ) delete m_valueSetter;
        if ( m_valueGetter ) delete m_valueGetter;
    }

    // Assignment

    PdmProxyValueField& operator=( const DataType& newFieldValue )
    {
        setValue( newFieldValue );
        return *this;
    }

    // Basic access

    void setValue( const DataType& fieldValue )
    {
        CAF_ASSERT( isInitializedByInitFieldMacro() );
        if ( m_valueSetter ) m_valueSetter->setValue( fieldValue );
    }
    DataType value() const
    {
        CAF_ASSERT( m_valueGetter );
        return m_valueGetter->getValue();
    }

    bool isStreamingField() const override { return is_vector<DataType>(); }
    bool hasGetter() const override { return m_valueGetter != nullptr; }
    bool hasSetter() const override { return m_valueSetter != nullptr; }

    // Implementation of PdmValueField interface

    QVariant toQVariant() const override
    {
        DataType val = value();
        return PdmValueFieldSpecialization<DataType>::convert( val );
    }
    void setFromQVariant( const QVariant& variant ) override
    {
        DataType val;
        PdmValueFieldSpecialization<DataType>::setFromVariant( variant, val );
        setValue( val );
    }
    bool isReadOnly() const override
    {
        if ( !m_valueSetter )
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    // Access operators

    DataType operator()() const { return value(); }
    DataType v() const { return value(); }
    bool     operator==( const DataType& otherValue ) const { return value() == otherValue; }

private:
    PDM_DISABLE_COPY_AND_ASSIGN( PdmProxyValueField );

    // Proxy Field stuff to handle the method pointers
    // The public registering methods must be written below the private classes
    // For some reason. Forward declaration did some weirdness.
private:
    class SetValueInterface
    {
    public:
        virtual ~SetValueInterface() {}
        virtual void setValue( const DataType& value ) = 0;
    };

    template <typename ObjectType>
    class SetterMethodCB : public SetValueInterface
    {
    public:
        typedef void ( ObjectType::*SetterMethodType )( const DataType& value );

        SetterMethodCB( ObjectType* obj, SetterMethodType setterMethod )
        {
            m_setterMethod = setterMethod;
            m_obj          = obj;
        }

        void setValue( const DataType& value ) { ( m_obj->*m_setterMethod )( value ); }

    private:
        SetterMethodType       m_setterMethod;
        PdmPointer<ObjectType> m_obj;
    };

    class GetValueInterface
    {
    public:
        virtual ~GetValueInterface() {}
        virtual DataType getValue() const = 0;
    };

    template <typename ObjectType>
    class GetterMethodCB : public GetValueInterface
    {
    public:
        typedef DataType ( ObjectType::*GetterMethodType )() const;

        GetterMethodCB( ObjectType* obj, GetterMethodType setterMethod )
        {
            m_getterMethod = setterMethod;
            m_obj          = obj;
        }

        DataType getValue() const { return ( m_obj->*m_getterMethod )(); }

    private:
        GetterMethodType       m_getterMethod;
        PdmPointer<ObjectType> m_obj;
    };

public:
    template <typename OwnerObjectType>
    void registerSetMethod( OwnerObjectType* obj, typename SetterMethodCB<OwnerObjectType>::SetterMethodType setterMethod )
    {
        if ( m_valueSetter ) delete m_valueSetter;
        m_valueSetter = new SetterMethodCB<OwnerObjectType>( obj, setterMethod );
    }

    template <typename OwnerObjectType>
    void registerGetMethod( OwnerObjectType* obj, typename GetterMethodCB<OwnerObjectType>::GetterMethodType getterMethod )
    {
        if ( m_valueGetter ) delete m_valueGetter;
        m_valueGetter = new GetterMethodCB<OwnerObjectType>( obj, getterMethod );
    }

private:
    SetValueInterface* m_valueSetter;
    GetValueInterface* m_valueGetter;
};

} // End of namespace caf
