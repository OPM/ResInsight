#pragma once

#include "cafPdmUiFieldHandle.h"

namespace caf
{



template < typename FieldType>
class PdmFieldUiCap : public PdmUiFieldHandle
{
public:
    PdmFieldUiCap(FieldType* field, bool giveOwnership) : PdmUiFieldHandle(field, giveOwnership) { m_field = field; }

    // Gui generalized interface
public:
    virtual QVariant    uiValue() const;
    virtual void        setValueFromUi(const QVariant& uiValue);
    virtual QList<PdmOptionItemInfo> valueOptions(bool* useOptionsOnly);

    virtual QVariant    toUiBasedQVariant() const;

private:
    QList<PdmOptionItemInfo> m_optionEntryCache;

private:
    FieldType* m_field;
};



// 
// Specialization for ChildFields to do nothing towards GUI
//
template < typename DataType>
class PdmFieldUiCap< PdmChildField<DataType*> > : public PdmUiFieldHandle
{
    typedef PdmChildField<DataType*> FieldType;
public:
    PdmFieldUiCap(FieldType* field, bool giveOwnership) : PdmUiFieldHandle(field, giveOwnership) { }

    // Gui generalized interface
public:
    virtual QVariant    uiValue() const { return QVariant();}
    virtual void        setValueFromUi(const QVariant& uiValue) { }
    virtual QList<PdmOptionItemInfo> valueOptions(bool* useOptionsOnly)  { return  QList<PdmOptionItemInfo>(); }

    virtual QVariant    toUiBasedQVariant() const { return QVariant(); }
};

//
// Specialization for ChildArrayFields to do nothing towards GUI
//
template < typename DataType>
class PdmFieldUiCap< PdmChildArrayField<DataType*> > : public PdmUiFieldHandle
{
    typedef PdmChildArrayField<DataType*> FieldType;
public:
    PdmFieldUiCap(FieldType* field, bool giveOwnership) : PdmUiFieldHandle(field, giveOwnership) { }

    // Gui generalized interface
public:
    virtual QVariant    uiValue() const { return QVariant(); }
    virtual void        setValueFromUi(const QVariant& uiValue) { }
    virtual QList<PdmOptionItemInfo> valueOptions(bool* useOptionsOnly)  { return  QList<PdmOptionItemInfo>(); }

    virtual QVariant    toUiBasedQVariant() const { return QVariant(); }
};

template<typename FieldType>
void AddUiCapabilityToField(FieldType* field)
{
    if(field->template capability< PdmFieldUiCap<FieldType> >() == NULL)
    {
        new PdmFieldUiCap<FieldType>(field, true);
    }
}


} // End of namespace caf

#include "cafInternalPdmUiFieldCapability.inl"
