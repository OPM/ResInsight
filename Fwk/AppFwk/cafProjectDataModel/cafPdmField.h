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


#pragma once

#include "cafPdmUiItem.h"
#include "cafPdmFieldImpl.h"

#include <set>
#include <assert.h>

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QTextStream>

namespace caf 
{

class PdmObject;
template <class T> class PdmPointer;
class PdmUiFieldEditorHandle;

//==================================================================================================
/// Base class for all fields, making it possible to handle them generically
//==================================================================================================

class PdmFieldHandle : public PdmUiItem
{
public:
    PdmFieldHandle() : m_isIOReadable(true), m_isIOWritable(true)   { m_ownerObject = NULL; m_keyword = "UNDEFINED"; }
    virtual ~PdmFieldHandle()                                       {  }

    virtual void     resetToDefaultValue()                          { };                           

    virtual void     readFieldData(QXmlStreamReader& xmlStream)  = 0;
    virtual void     writeFieldData(QXmlStreamWriter& xmlStream) = 0;

    bool             isIOReadable()                                 { return m_isIOReadable; }
    bool             isIOWritable()                                 { return m_isIOWritable; }
    void             setIOWritable(bool isWritable)                 {  m_isIOWritable = isWritable; }
    void             setIOReadable(bool isReadable)                 {  m_isIOReadable = isReadable; }

    void             setKeyword(const QString& keyword);
    QString          keyword() const                                { return m_keyword;    }

    void             setOwnerObject(PdmObject * owner)              { m_ownerObject = owner; }
    PdmObject*       ownerObject()                                  { return m_ownerObject;  }

    // Generalized access methods for User interface

    virtual QVariant uiValue() const                                { return QVariant(); }
    virtual void     setValueFromUi(const QVariant& )               {  }

    virtual bool     hasChildObjects();
    virtual void     childObjects(std::vector<PdmObject*>* )        {  }
    virtual void     removeChildObject(PdmObject* )                 {  }

    virtual QList<PdmOptionItemInfo> 
                     valueOptions( bool* useOptionsOnly)            {  return  QList<PdmOptionItemInfo>(); }

protected:
    bool             assertValid() const;
protected:
    PdmObject*       m_ownerObject;
private:
    QString          m_keyword;
    bool             m_isIOReadable;
    bool             m_isIOWritable;

};


//==================================================================================================
/// Field class encapsulating data with input and output of this data to/from a QXmlStream
/// read/write-FieldData is supposed to be specialized for types needing specialization
//==================================================================================================
template <typename T> struct PdmFieldWriter;
template <typename T> struct PdmFieldReader;

template<typename DataType >
class PdmField : public PdmFieldHandle
{
public:
    PdmField() {}
    virtual ~PdmField() {}

    // Copy and assignment must ignore the default value.
    PdmField(const PdmField& other) : PdmFieldHandle()              { assertValid(); m_fieldValue = other.m_fieldValue; }
    PdmField(const DataType& fieldValue) : PdmFieldHandle()         { assertValid(); m_fieldValue = fieldValue; }
    PdmField&       operator= (const PdmField& other)               { assertValid(); m_fieldValue = other.m_fieldValue; return *this; }
    PdmField&       operator= (const DataType& fieldValue)          { assertValid(); m_fieldValue = fieldValue; return *this; }

                    operator DataType () const                      { return m_fieldValue; }

   // DataType&       operator()()                                    { assertValid(); return m_fieldValue; }
    const DataType& operator()() const                              { return m_fieldValue; }
    DataType&       v()                                             { assertValid(); return m_fieldValue; }
    const DataType& v() const                                       { return m_fieldValue; }

    bool            operator== (const DataType& fieldValue) const   { return m_fieldValue == fieldValue; }

    // readFieldData assumes that the xmlStream points to first token of field content.
    // After reading, the xmlStream is supposed to point to the first token after the field content.
    // (typically an "endElement")

    virtual void    readFieldData(QXmlStreamReader& xmlStream)      { PdmFieldReader<DataType>::readFieldData(*this, xmlStream); }
    virtual void    writeFieldData(QXmlStreamWriter& xmlStream)     { PdmFieldWriter<DataType>::writeFieldData(*this, xmlStream);}
    
    const DataType& defaultValue() const                            { return m_defaultFieldValue; }
    void            setDefaultValue(const DataType& val)            { m_defaultFieldValue = val; }
    virtual void    resetToDefaultValue()                           { m_fieldValue = m_defaultFieldValue; }

    // Gui generalized interface
    virtual QVariant    uiValue() const;
    virtual void        setValueFromUi(const QVariant& uiValue);

    virtual QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly);
    virtual void        childObjects(std::vector<PdmObject*>* objects) { PdmFieldTypeSpecialization<DataType>::childObjects(*this, objects); }

protected:
    DataType m_fieldValue;
    DataType m_defaultFieldValue;
    QList<PdmOptionItemInfo> m_optionEntryCache;
};


//==================================================================================================
/// Specialization for pointers, but only applicable to PdmObjectBase derived objects.
/// The pointer is guarded, meaning that it will be set to NULL if the object pointed to 
/// is deleted. The referenced object will be printed in place in the xml-file
//==================================================================================================

template<typename DataType >
class PdmField <DataType*> : public PdmFieldHandle
{
    typedef DataType* DataTypePtr;
public:
    PdmField()  : PdmFieldHandle()                                          { m_fieldValue = NULL; }
    PdmField(const PdmField& other);
    PdmField(const DataTypePtr& fieldValue);
    virtual ~PdmField();

    PdmField&                   operator= (const PdmField& other);
    PdmField&                   operator= (const DataTypePtr & fieldValue);

                                operator DataType* () const                 { return m_fieldValue; }
    DataType* 	                operator->() const                          { return m_fieldValue; }

    const PdmPointer<DataType>& operator()() const                          { return m_fieldValue; }
    const PdmPointer<DataType>& v() const                                   { return m_fieldValue; }

    bool                        operator==(const DataTypePtr& fieldValue)   { return m_fieldValue == fieldValue; }

    // readFieldData assumes that the xmlStream points to first token of field content.
    // After reading, the xmlStream is supposed to point to the first token after the field content.
    // (typically an "endElement")
    virtual void                readFieldData(QXmlStreamReader& xmlStream);
    virtual void                writeFieldData(QXmlStreamWriter& xmlStream);   

    const DataTypePtr&          defaultValue() const                        { return NULL; }
    void                        setDefaultValue(const DataTypePtr& )        { }

    // Gui generalized methods
    virtual QVariant            uiValue() const                             { return QVariant();}
    virtual void                childObjects(std::vector<PdmObject*>* objects);

protected:
    PdmPointer<DataType> m_fieldValue;
};

//==================================================================================================
/// PdmFieldClass to handle a collection of PdmObject derived pointers
/// The reasons for this class is to add itself as parentField into the objects being pointed to.
/// The interface is made similar to std::vector<>, and the complexity of the methods is similar too.
//==================================================================================================

template<typename DataType>
class PdmPointersField : public PdmFieldHandle
{
public:
    PdmPointersField()                                                              
    { bool doNotUsePdmPointersFieldForAnythingButPointersToPdmObject = false; assert(doNotUsePdmPointersFieldForAnythingButPointersToPdmObject);  }
};

template<typename DataType>
class PdmPointersField<DataType*> : public PdmFieldHandle
{
    typedef DataType* DataTypePtr;
public:
    PdmPointersField()          { }
    PdmPointersField(const PdmPointersField& other);
    virtual ~PdmPointersField();

    PdmPointersField&   operator= (const PdmPointersField& other);
    bool                operator==(const PdmPointersField& other) { return m_pointers == other.m_pointers; } 
    PdmPointersField&   operator() () { return *this; }

    size_t              size() const                              { return m_pointers.size(); }
    bool                empty() const                             { return m_pointers.empty(); }
    DataType*           operator[] (size_t index) const;

    void                push_back(DataType* pointer);
    void                set(size_t index, DataType* pointer);
    void                insert(size_t indexAfter, DataType* pointer);
    void                insert(size_t indexAfter, const std::vector<PdmPointer<DataType> >& objects);
    size_t              count(const DataType* pointer) const;

    void                clear();
    void                erase(size_t index);
    void                deleteAllChildObjects();

    // Reimplementation of PdmFieldhandle methods
    virtual void        readFieldData(QXmlStreamReader& xmlStream);
    virtual void        writeFieldData(QXmlStreamWriter& xmlStream);   

    // Gui generalized methods
    virtual void        childObjects(std::vector<PdmObject*>* objects);
    virtual void        removeChildObject(PdmObject* object);

private:
    void                removeThisAsParentField();
    void                addThisAsParentField();

private:
    std::vector< PdmPointer<DataType> > m_pointers;
};


} // End of namespace caf

#include "cafPdmField.inl"
