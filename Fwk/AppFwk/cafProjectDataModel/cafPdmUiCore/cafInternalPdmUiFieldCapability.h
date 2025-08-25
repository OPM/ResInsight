#pragma once

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmUiFieldHandle.h"

namespace caf
{
template <typename FieldType>
class PdmFieldUiCap : public PdmUiFieldHandle
{
public:
    PdmFieldUiCap( FieldType* field, bool giveOwnership )
        : PdmUiFieldHandle( field, giveOwnership )
    {
        m_field = field;
    }

    // Gui generalized interface
public:
    QVariant                 uiValue() const override;
    void                     setValueFromUiEditor( const QVariant& uiValue, bool notifyFieldChanged ) override;
    QList<PdmOptionItemInfo> valueOptions() const override;

    QVariant toUiBasedQVariant() const override;

private:
    bool isQVariantDataEqual( const QVariant& oldUiBasedQVariant, const QVariant& newUiBasedQVariant ) const override;

    mutable QList<PdmOptionItemInfo> m_optionEntryCache;

private:
    FieldType* m_field;
};

//
// Specialization for ChildFields to do nothing towards GUI
//
template <typename DataType>
class PdmFieldUiCap<PdmChildField<DataType*>> : public PdmUiFieldHandle
{
    typedef PdmChildField<DataType*> FieldType;

public:
    PdmFieldUiCap( FieldType* field, bool giveOwnership )
        : PdmUiFieldHandle( field, giveOwnership )
    {
        // In almost all use cases, we want to hide the visual appearance of the child field.
        // The object contained in the child field will be visible in the project tree as a child of the
        // parent object.
        setUiTreeHidden( true );
    }

    // Gui generalized interface
public:
    QVariant                 uiValue() const override { return QVariant(); }
    void                     setValueFromUiEditor( const QVariant& uiValue, bool notifyFieldChanged ) override {}
    QList<PdmOptionItemInfo> valueOptions() const override { return QList<PdmOptionItemInfo>(); }

    QVariant toUiBasedQVariant() const override { return QVariant(); }
};

//
// Specialization for ChildArrayFields to do nothing towards GUI
//
template <typename DataType>
class PdmFieldUiCap<PdmChildArrayField<DataType*>> : public PdmUiFieldHandle
{
    typedef PdmChildArrayField<DataType*> FieldType;

public:
    PdmFieldUiCap( FieldType* field, bool giveOwnership )
        : PdmUiFieldHandle( field, giveOwnership )
    {
        // In almost all use cases, we want to hide the visual appearance of the child array field.
        // They are usually members of a collection object, and this object is usually visible in the project tree
        // The objects contained in the child array field will be visible in the project tree as children of the
        // collection object
        setUiTreeHidden( true );
    }

    // Gui generalized interface
public:
    QVariant                 uiValue() const override { return QVariant(); }
    void                     setValueFromUiEditor( const QVariant& uiValue, bool notifyFieldChanged ) override {}
    QList<PdmOptionItemInfo> valueOptions() const override { return QList<PdmOptionItemInfo>(); }

    QVariant toUiBasedQVariant() const override { return QVariant(); }
};

template <typename FieldType>
void addUiCapabilityToField( FieldType* field )
{
    if ( field->template capability<PdmFieldUiCap<FieldType>>() == NULL )
    {
        new PdmFieldUiCap<FieldType>( field, true );
    }
}

template <typename FieldType>
void configureCapabilities( FieldType* field )
{
    field->configureCapabilities();
}

} // End of namespace caf

#include "cafInternalPdmUiFieldCapability.inl"
