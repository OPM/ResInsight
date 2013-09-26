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
#include <QVariant>
#include <QList>
#include <QStringList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QTextStream>

#include "cafPdmUiItem.h"
#include "cafPdmObjectFactory.h"

namespace caf 
{

// Forward declarations
template <typename T> class PdmField;
template <typename T> class PdmPointer;
template <typename T> class AppEnum;

//==================================================================================================
/// A proxy class that implements the Gui interface of fields
///
/// This class collects methods that need specialization when introducing a new type in a PdmField.
/// Having those methods in a separate class makes it possible to "partially specialize" the methods
/// for container classes etc. since partial specialization of template functions is not C++ as of yet.
///
/// When introducing a new type in a PdmField, you might need to implement a (partial)specialization 
/// of this class.
//==================================================================================================

template <typename T>
class PdmFieldTypeSpecialization
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert(const T& value)
    {
        return QVariant(value);
    }

    /// Set the field value from a QVariant
    static void setFromVariant(const QVariant& variantValue, T& value)
    {
        value = variantValue.value<T>();
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const T& )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Methods to retrieve the possible PdmObject pointed to by a field
    static void childObjects(const PdmField<T>& , std::vector<PdmObject*>* )    
    { }
    /*
    static void    writeFieldData(PdmField<T> & field, QXmlStreamWriter& xmlStream)
    {
        QString dataString; 
        QTextStream data(&dataString, QIODevice::WriteOnly); 
        data << field.v(); 
        xmlStream.writeCharacters(dataString);
    }

    static void readFieldData(PdmField<DataType> & field, QXmlStreamReader& xmlStream)
    {
        PdmFieldHandle::skipComments(xmlStream);
        if (!xmlStream.isCharacters()) return;

        QString dataString = xmlStream.text().toString();
        QTextStream data(&dataString, QIODevice::ReadOnly);  
        data >> field.v();

        // Make stream point to end of element
        QXmlStreamReader::TokenType type;
        type = xmlStream.readNext();
        PdmFieldHandle::skipCharactersAndComments(xmlStream);
    }
    */
};

//==================================================================================================
/// Partial specialization for PdmField< std::list<T> >
//==================================================================================================

template <typename T>
class PdmFieldTypeSpecialization < std::list<T> >
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert(const std::list<T>& value)
    {
        QList<QVariant> returnList;
        typename std::list<T>::const_iterator it;
        for (it = value.begin(); it != value.end() ; ++it)
        {
            returnList.push_back(QVariant(*it));
        }
        return returnList;
    }


    /// Set the field value from a QVariant
    static void setFromVariant(const QVariant& variantValue, std::list<T>& value)
    {
        if (variantValue.canConvert< QList<QVariant> >())
        {
            value.clear();
            QList<QVariant> lst = variantValue.toList();
            int i;
            for (i = 0; i < lst.size(); ++i)
            {
                value.push_back(lst[i].value<T>());
            }
        }
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const std::list<T>& )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Methods to retrieve the possible PdmObject pointed to by a field
    static void childObjects(const PdmField< std::list<T> >& , std::vector<PdmObject*>* )    
    { }

};

//==================================================================================================
/// Partial specialization for PdmField< std::list< PdmPointer<T> > >
//==================================================================================================

template <typename T>
class PdmFieldTypeSpecialization < std::list<PdmPointer<T> > >
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert(const std::list<PdmPointer<T> >& )
    {
        return QVariant(); // Do nothing. The members are "children"
    }

    /// Set the field value from a QVariant
    // Overloaded to do nothing because this is supposed to be handled as children
    static void setFromVariant(const QVariant& , std::list<PdmPointer<T> >& )     
    {   }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const std::list<PdmPointer<T> >& )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Methods to retrieve the possible PdmObject pointed to by a field
    static void childObjects(const PdmField<std::list<PdmPointer<T> > >& field, std::vector<PdmObject*>* objects)
    {
        if (!objects) return;

        typename std::list<PdmPointer<T> >::const_iterator it;
        for (it = field.v().begin() ; it != field.v().end(); ++it)
        {
            objects->push_back(*it);
        }
    }

};

//==================================================================================================
/// Partial specialization for PdmField< std::vector<T> >
//==================================================================================================

template <typename T>
class PdmFieldTypeSpecialization < std::vector<T> >
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert(const std::vector<T>& value)
    {
        QList<QVariant> returnList;
        typename std::vector<T>::const_iterator it;
        for (it = value.begin(); it != value.end() ; ++it)
        {
            returnList.push_back(QVariant(*it));
        }
        return returnList;
    }

    /// Set the field value from a QVariant
    static void setFromVariant(const QVariant& variantValue, std::vector<T>& value)
    {
        if (variantValue.canConvert< QList<QVariant> >())
        {
            value.clear();
            QList<QVariant> lst = variantValue.toList();
            int i;
            for (i = 0; i < lst.size(); ++i)
            {
                value.push_back(lst[i].value<T>());
            }
        }
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const  std::vector<T>& )
    {
        return QList<PdmOptionItemInfo>();
    }

    /// Methods to retrieve the possible PdmObject pointed to by a field
    static void childObjects(const PdmField< std::vector<T> > & field, std::vector<PdmObject*>* objects)
    { }

};

//==================================================================================================
/// Partial specialization for PdmField<  caf::AppEnum<T> >
//==================================================================================================

template <typename T>
class PdmFieldTypeSpecialization < caf::AppEnum<T> >
{
public:
    /// Convert the field value into a QVariant
    static QVariant convert(const caf::AppEnum<T>& value)
    {
        return QVariant(static_cast<unsigned int>(caf::AppEnum<T>::index(value)));
    }

    /// Set the field value from a QVariant
    static void setFromVariant(const QVariant& variantValue, caf::AppEnum<T>& value)
    {
        value.setFromIndex(variantValue.toInt());
    }

    /// Methods to get a list of options for a field, specialized for AppEnum
    static QList<PdmOptionItemInfo> valueOptions( bool* useOptionsOnly, const caf::AppEnum<T>& )
    {
        if (useOptionsOnly) *useOptionsOnly = true;

        QStringList optionTexts = caf::AppEnum<T>::uiTexts();
        QList<PdmOptionItemInfo> optionList;
        int i;
        for (i = 0; i < optionTexts.size(); ++i)
        {
            optionList.push_back(PdmOptionItemInfo(optionTexts[i], static_cast<unsigned int>(i)));
        }

        return optionList;
    }

    /// Methods to retrieve the possible PdmObject pointed to by a field
    static void childObjects(const PdmField< caf::AppEnum<T> >& field, std::vector<PdmObject*>* objects)
    { }

};

class PdmFieldIOHelper
{
public:
    // Utility functions for reading from QXmlStreamReader
    static void      skipCharactersAndComments(QXmlStreamReader& xmlStream);
    static void      skipComments(QXmlStreamReader& xmlStream);

};

//--------------------------------------------------------------------------------------------------
/// Generic write method for fields. Will work as long as DataType supports the stream operator 
/// towards a QTextStream. Some special datatype should not specialize this method unless it is 
/// impossible/awkward to implement the stream operator
/// Implemented in a proxy class to allow  partial specialization 
//--------------------------------------------------------------------------------------------------
template <typename T>
struct PdmFieldWriter
{
    static void    writeFieldData(PdmField<T> & field, QXmlStreamWriter& xmlStream)
    {
        QString dataString; 
        QTextStream data(&dataString, QIODevice::WriteOnly); 
        data << field.v(); 
        xmlStream.writeCharacters(dataString);
    }
};

template <typename T>
struct PdmFieldReader
{
    static void    readFieldData(PdmField<T> & field, QXmlStreamReader& xmlStream);
};

//--------------------------------------------------------------------------------------------------
/// Generic read method for fields. Will work as long as DataType supports the stream operator 
/// towards a QTextStream. Some special datatype should not specialize this method unless it is 
/// impossible/awkward to implement the stream operator
//--------------------------------------------------------------------------------------------------
template<typename DataType >
void PdmFieldReader<DataType>::readFieldData(PdmField<DataType> & field, QXmlStreamReader& xmlStream)
{
    PdmFieldIOHelper::skipComments(xmlStream);
    if (!xmlStream.isCharacters()) return;

    QString dataString = xmlStream.text().toString();
    QTextStream data(&dataString, QIODevice::ReadOnly);  
    data >> field.v();

    // Make stream point to end of element
    QXmlStreamReader::TokenType type;
    type = xmlStream.readNext();
    PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
}

//--------------------------------------------------------------------------------------------------
/// Specialized read function for QStrings, because the >> operator only can read word by word
//--------------------------------------------------------------------------------------------------
template<>
void PdmFieldReader<QString>::readFieldData(PdmField<QString> & field, QXmlStreamReader& xmlStream);

//==================================================================================================
/// Read and write method specializations for containers of pointers
/// std::list is the main one
//==================================================================================================

template <typename T>
struct PdmFieldWriter<std::list< PdmPointer<T> > >
{
    static void    writeFieldData(PdmField<std::list< PdmPointer<T> > > & field, QXmlStreamWriter& xmlStream)
    {
        typename std::list< PdmPointer<T> >::iterator it;
        for (it = field.v().begin(); it != field.v().end(); ++it)
        {
            if (*it == NULL) continue;

            QString className = (*it)->classKeyword();

            xmlStream.writeStartElement("", className);
            (*it)->writeFields(xmlStream);
            xmlStream.writeEndElement();
        }
    }
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
struct PdmFieldReader<std::list< PdmPointer<T> > >
{
    static void    readFieldData(PdmField<std::list< PdmPointer<T> > > & field, QXmlStreamReader& xmlStream)
    {
        T * currentObject = NULL;
        PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
        while (xmlStream.isStartElement())
        {
            QString className = xmlStream.name().toString();

            PdmObject * obj = PdmObjectFactory::instance()->create(className);

            if (obj == NULL)
            {
                // Warning: Unknown className read
                // Skip to corresponding end element
                xmlStream.skipCurrentElement();
                PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
                continue; 
            }

            currentObject = dynamic_cast<T *> (obj);

            if (currentObject == NULL)
            {
                // Warning: Inconsistency in factory !! Assert ?
                // Skip to corresponding end element
                xmlStream.skipCurrentElement();
                PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
                continue; 
            }

            currentObject->readFields(xmlStream);
            field.v().push_back(currentObject);

            // Skip comments and for some reason: Characters. The last bit should not be correct, 
            // but Qt reports a character token between EndElement and StartElement

            QXmlStreamReader::TokenType type;
            type = xmlStream.readNext();
            PdmFieldIOHelper::skipCharactersAndComments(xmlStream);
        }
    }
};


} // End of namespace caf


//==================================================================================================
/// QTextStream Stream operator overloading for bool`s
/// Prints bool`s as "True"/"False", and reads them too
//==================================================================================================

QTextStream& operator >> (QTextStream& str, bool& value);
QTextStream& operator << (QTextStream& str, const bool& value);


//==================================================================================================
/// QTextStream Stream operator overloading for QDateTimes`s
/// 
//==================================================================================================
//class QDateTime;
QTextStream&  operator >> (QTextStream& str, QDateTime& value);
QTextStream&  operator << (QTextStream& str, const QDateTime& value);


//==================================================================================================
/// QTextStream Stream operator overloading for std::vector of things.
/// Makes automated IO of PdmField< std::vector< Whatever > possible as long as
/// the type will print as one single word
//==================================================================================================

template < typename T >
QTextStream& operator << (QTextStream& str, const std::vector<T>& sobj)
{
    size_t i;
    for (i = 0; i < sobj.size(); ++i)
    {
        str << sobj[i] << " ";
    }
    return str;
}

template < typename T >
QTextStream& operator >> (QTextStream& str, std::vector<T>& sobj)
{
    while (str.status() == QTextStream::Ok )
    {   
        T d;
        str >> d;
        if (str.status() == QTextStream::Ok ) sobj.push_back(d);
    }
    return str;
}
