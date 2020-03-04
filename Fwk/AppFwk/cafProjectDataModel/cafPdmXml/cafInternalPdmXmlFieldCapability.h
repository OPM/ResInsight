#pragma once

#include "cafInternalPdmXmlFieldReaderWriter.h"
#include "cafPdmXmlFieldHandle.h"

#include <typeinfo>

namespace caf
{

template < typename FieldType>
class PdmFieldXmlCap : public PdmXmlFieldHandle
{
public:
    PdmFieldXmlCap(FieldType* field, bool giveOwnership) : PdmXmlFieldHandle(field, giveOwnership)
    {
        m_field = field;
        m_dataTypeName = QString("%1").arg(typeid(FieldType).name());
    }

    // Xml Serializing
public:
    void        readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory) override;
    void        writeFieldData(QXmlStreamWriter& xmlStream) const override;
    bool        resolveReferences() override;

    bool        isVectorField() const;

private:
    FieldType* m_field;
};

template <typename DataType> class PdmPtrField;

template < typename DataType>
class PdmFieldXmlCap< PdmPtrField<DataType*> > : public PdmXmlFieldHandle
{
    typedef PdmPtrField<DataType*> FieldType;
public:
    PdmFieldXmlCap(FieldType* field, bool giveOwnership) : PdmXmlFieldHandle(field, giveOwnership)
    { 
        m_field = field;
        m_dataTypeName = DataType::classKeywordStatic();
        m_isResolved = false;
        m_referenceString = "";
    }

    // Xml Serializing
public:
    void        readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory) override;
    void        writeFieldData(QXmlStreamWriter& xmlStream) const override;
    bool        resolveReferences() override;
    QString     referenceString() const override;

private:
    FieldType* m_field;

    // Resolving
    QString                     m_referenceString;
    bool                        m_isResolved;
};

template <typename DataType> class PdmPtrArrayField;

template < typename DataType>
class PdmFieldXmlCap< PdmPtrArrayField<DataType*> >: public PdmXmlFieldHandle
{
    typedef PdmPtrArrayField<DataType*> FieldType;
public:
    PdmFieldXmlCap(FieldType* field, bool giveOwnership) : PdmXmlFieldHandle(field, giveOwnership)
    {
        m_field = field;
        m_dataTypeName = DataType::classKeywordStatic();
        m_isResolved = false;
        m_referenceString = "";
    }

    // Xml Serializing
public:
    void        readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory) override;
    void        writeFieldData(QXmlStreamWriter& xmlStream) const override;
    bool        resolveReferences() override;

private:
    FieldType* m_field;

    // Resolving
    QString                     m_referenceString;
    bool                        m_isResolved;
};


template <typename DataType> class PdmChildField;

template < typename DataType>
class PdmFieldXmlCap< PdmChildField<DataType*> > : public PdmXmlFieldHandle
{
    typedef PdmChildField<DataType*> FieldType;
public:
    PdmFieldXmlCap(FieldType* field, bool giveOwnership) : PdmXmlFieldHandle(field, giveOwnership) { m_field = field; m_dataTypeName = DataType::classKeywordStatic(); }

    // Xml Serializing
public:
    void        readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory) override; 
    void        writeFieldData(QXmlStreamWriter& xmlStream) const override;
    bool        resolveReferences() override;

private:
    FieldType* m_field;
};


template <typename DataType> class PdmChildArrayField;

template < typename DataType>
class PdmFieldXmlCap< PdmChildArrayField<DataType*> > : public PdmXmlFieldHandle
{
    typedef PdmChildArrayField<DataType*> FieldType;
public:
    PdmFieldXmlCap(FieldType* field, bool giveOwnership) : PdmXmlFieldHandle(field, giveOwnership) { m_field = field; m_dataTypeName = DataType::classKeywordStatic();}

    // Xml Serializing
public:
    void        readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory) override;
    void        writeFieldData(QXmlStreamWriter& xmlStream) const override;
    bool        resolveReferences() override;
private:
    FieldType* m_field;
};




template<typename FieldType>
void AddXmlCapabilityToField(FieldType* field)
{
    if(field->template capability< PdmFieldXmlCap<FieldType> >() == NULL)
    {
        new PdmFieldXmlCap<FieldType>(field, true);
    }
}


template<typename FieldType>
void RegisterClassWithField(const QString& classKeyword, FieldType* field)
{
    field->setOwnerClass(classKeyword);
}


} // End of namespace caf

#include "cafInternalPdmXmlFieldCapability.inl"
