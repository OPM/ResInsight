//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include "cafPdmUiItem.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmPointer.h"

#include <set>
#include <assert.h>

class QXmlStreamReader;
class QXmlStreamWriter;


// Taken from gtest.h
//
// Due to C++ preprocessor weirdness, we need double indirection to
// concatenate two tokens when one of them is __LINE__.  Writing
//
//   foo ## __LINE__
//
// will result in the token foo__LINE__, instead of foo followed by
// the current line number.  For more details, see
// http://www.parashift.com/c++-faq-lite/misc-technical-issues.html#faq-39.6
#define PDM_OBJECT_STRING_CONCATENATE(foo, bar) PDM_OBJECT_STRING_CONCATENATE_IMPL_(foo, bar)
#define PDM_OBJECT_STRING_CONCATENATE_IMPL_(foo, bar) foo ## bar


namespace caf 
{

class PdmFieldHandle;
template < class FieldDataType > class PdmField;
class PdmUiEditorAttribute;

//==================================================================================================
/// Macros helping in development of PDM objects
//==================================================================================================

/// CAF_PDM_HEADER_INIT assists the factory used when reading objects from file
/// Place this in the header file inside the class definition of your PdmObject

#define CAF_PDM_HEADER_INIT \
public: \
    virtual QString classKeyword()   { return  classKeywordStatic(); } \
    static  QString classKeywordStatic(); \
    static  bool Error_You_forgot_to_add_the_macro_CAF_PDM_HEADER_INIT_and_or_CAF_PDM_SOURCE_INIT_to_your_cpp_file_for_this_class()

/// CAF_PDM_SOURCE_INIT associates the file keyword used for storage with the class and initializes the factory
/// Place this in the cpp file, preferably above the constructor

#define CAF_PDM_SOURCE_INIT(ClassName, keyword) \
    bool    ClassName::Error_You_forgot_to_add_the_macro_CAF_PDM_HEADER_INIT_and_or_CAF_PDM_SOURCE_INIT_to_your_cpp_file_for_this_class() { return false;} \
    QString ClassName::classKeywordStatic() { assert(PdmObject::isValidXmlElementName(keyword)); return keyword;   } \
    static bool PDM_OBJECT_STRING_CONCATENATE(pdm_object_factory_init_, __LINE__) = caf::PdmObjectFactory::instance()->registerCreator<ClassName>() 

/// InitObject sets up the user interface related information for the object
/// Placed in the constructor of your PdmObject

#define CAF_PDM_InitObject(uiName, iconResourceName, toolTip, whatsThis) \
{ \
    static caf::PdmUiItemInfo objDescr(uiName, QIcon(QString(iconResourceName)), toolTip, whatsThis); \
    setUiItemInfo(&objDescr); \
}

/// InitField sets the file keyword for the field, 
/// adds the field to the internal data structure in the PdmObject, 
/// sets the default value for the field, 
/// and sets up the static user interface related information for the field

#define CAF_PDM_InitField(field, keyword, default, uiName, iconResourceName, toolTip, whatsThis) \
{ \
    static bool chekingThePresenceOfHeaderAndSourceInitMacros = Error_You_forgot_to_add_the_macro_CAF_PDM_HEADER_INIT_and_or_CAF_PDM_SOURCE_INIT_to_your_cpp_file_for_this_class(); \
     \
    static caf::PdmUiItemInfo objDescr(uiName, QIcon(QString(iconResourceName)), toolTip, whatsThis); \
    addField(field, keyword, default, &objDescr); \
}

/// InitFieldNoDefault does the same as InitField but omits the default value.

#define CAF_PDM_InitFieldNoDefault(field, keyword, uiName, iconResourceName, toolTip, whatsThis) \
{ \
    static bool chekingThePresenceOfHeaderAndSourceInitMacros = Error_You_forgot_to_add_the_macro_CAF_PDM_HEADER_INIT_and_or_CAF_PDM_SOURCE_INIT_to_your_cpp_file_for_this_class(); \
    \
    static caf::PdmUiItemInfo objDescr(uiName, QIcon(QString(iconResourceName)), toolTip, whatsThis); \
    addFieldNoDefault(field, keyword, &objDescr); \
}


//==================================================================================================
/// The base class of all objects that will use the features of the field based IO
/// Inherit this class to make your Pdm-based data structure
//==================================================================================================

class PdmObject : public PdmUiItem
{
public:
    PdmObject() { }
    virtual ~PdmObject();

    /// The classKeyword method is overridden in subclasses by the CAF_PDM_HEADER_INIT macro
    virtual QString         classKeyword() = 0;

    void                    readFields (QXmlStreamReader& inputStream );
    void                    writeFields(QXmlStreamWriter& outputStream);

    /// The registered fields contained in this PdmObject. Registered by subclasses.
    void                    fields(std::vector<PdmFieldHandle*>& fields) const;
    /// The fields containing pointers to this PdmObject. Use ownerObject() on the fieldHandle to get the PdmObject parent.
    void                    parentFields(std::vector<PdmFieldHandle*>& fields) const;
    /// Remove pointer to this from all parent fields
    void                    removeFromParentFields();

    /// 
    void                    parentObjects(std::vector<PdmObject*>& objects) const;

    /// 
    template <typename T>
    void                    parentObjectsOfType(std::vector<T*>& objects) const;

    /// Method to be called from the Ui classes creating Auto Gui to get the group information 
    /// supplied by the \sa defineUiOrdering method that can be reimplemented
    void                    uiOrdering(QString uiConfigName, PdmUiOrdering& uiOrdering) const;

    /// For a specific field, return editor specific parameters used to customize the editor behavior..
    void                    editorAttribute(const PdmFieldHandle* field, QString uiConfigName, PdmUiEditorAttribute * attribute);

    // Virtual interface to override in subclasses to support special behaviour if needed
public: // Virtual 
    virtual PdmFieldHandle* userDescriptionField() { return NULL; }
    /// Method to reimplement to catch when the field has changed due to setUiValue()
    virtual void            fieldChangedByUi(const PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) {}
    /// Method to re-implement to supply option values for a specific field
    virtual QList<PdmOptionItemInfo> 
                            calculateValueOptions(const PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) { return QList<PdmOptionItemInfo>(); }
    // For later // virtual void editorAttributeChangedByUI(const PdmFieldHandle* field, QString uiConfigName, const PdmUiAttributeHandle * attributes);


public:
    /// Check if a string is a valid Xml element name
    static bool isValidXmlElementName(const QString& name);

protected: // Virtual 
    /// Method gets called from PdmDocument after all objects are read. 
    /// Re-implement to set up internal pointers etc. in your data structure
    virtual void            initAfterRead() {};
    /// Method gets called from PdmDocument before saving document. 
    /// Re-implement to make sure your fields have correct data before saving
    virtual void            setupBeforeSave() {};

    /// Override to customize the order and grouping of the Gui.
    /// Fill up the uiOrdering object with groups and field references to create the gui structure
    /// If the uiOrdering is empty, it is interpreted as meaning all fields w/o grouping.
    virtual void            defineUiOrdering(QString uiConfigName, PdmUiOrdering& uiOrdering) const {}

    /// Override to provide editor specific data for the field and uiConfigName 
    virtual void            defineEditorAttribute(const PdmFieldHandle* field, QString uiConfigName, PdmUiEditorAttribute * attribute) {}

public:
    /// operator= implemented to avoid copying the internal m_fields
    PdmObject&              operator=(const PdmObject& ) { return *this; }

protected: 
    /// Adds field to the internal data structure and sets the file keyword and Ui information 
    /// Consider this method private. Please use the CAF_PDM_InitField() macro instead
    template< typename FieldDataType >
    void addField(PdmField<FieldDataType>* field, const QString& keyword, const FieldDataType& defaultValue, PdmUiItemInfo * fieldDescription)
    {
        addFieldNoDefault(field, keyword, fieldDescription);
        field->setDefaultValue(defaultValue);
        *field = defaultValue;
    }

    /// Does the same as the above method, but omits the default value.
    /// Consider this method private. Please use the CAF_PDM_InitFieldNoDefault() macro instead.
    void addFieldNoDefault(PdmFieldHandle* field, const QString& keyword, PdmUiItemInfo * fieldDescription);

private:
    // Copy and assignment operators are implemented to avoid copying the internal management data structures.

    // If you use copy constructor in your application code, the compiler reports
    // "error C2248: 'caf::PdmObjectBase::PdmObjectBase' : cannot access private member declared in ..."
    // To fix this issue, implement a public copy constructor in your derived class.
    PdmObject(const PdmObject& ): PdmUiItem() { }
    PdmFieldHandle*               findField(const QString& keyword);

    template < class T > friend class PdmField;
    template < class T > friend class PdmPointersField;

    friend class PdmDocument;
    friend class PdmObjectGroup;

    void                            addParentField(PdmFieldHandle* parentField);
    void                            removeParentField(PdmFieldHandle* parentField);

private:
    std::multiset<PdmFieldHandle*>  m_parentFields;
    std::vector<PdmFieldHandle*>    m_fields;

    // Support system for PdmPointer
    friend class PdmPointerImpl;
    std::set<PdmObject**>         m_pointersReferencingMe;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
template <typename T>
void PdmObject::parentObjectsOfType(std::vector<T*>& objects) const
{
    std::vector<PdmObject*> parents;
    this->parentObjects(parents);

    for (size_t i = 0; i < parents.size(); i++)
    {
        T* objectOfType = dynamic_cast<T*>(parents[i]);
        if (objectOfType)
        {
            objects.push_back(objectOfType);
        }
    }
}


} // End of namespace caf
