
#include "cafPdmObjectHandle.h"

#include "cafAssert.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObjectCapability.h"


namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectHandle::~PdmObjectHandle()
{
    this->prepareForDelete();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString PdmObjectHandle::classKeywordStatic()
{
    return classKeywordAliases().front();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> PdmObjectHandle::classKeywordAliases()
{
    return { QString("PdmObjectHandle") };
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::fields(std::vector<PdmFieldHandle*>& fields) const
{
    fields = m_fields;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::setAsParentField(PdmFieldHandle* parentField)
{
    CAF_ASSERT(m_parentField == nullptr);

    m_parentField = parentField;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::removeAsParentField(PdmFieldHandle* parentField)
{
    CAF_ASSERT(m_parentField == parentField);

    m_parentField = nullptr;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::addReferencingPtrField(PdmFieldHandle* fieldReferringToMe)
{
    if (fieldReferringToMe != nullptr) m_referencingPtrFields.insert(fieldReferringToMe);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::removeReferencingPtrField(PdmFieldHandle* fieldReferringToMe)
{
    if (fieldReferringToMe != nullptr) m_referencingPtrFields.erase(fieldReferringToMe);
}

//--------------------------------------------------------------------------------------------------
/// Appends pointers to all the PdmPtrFields containing a pointer to this object.
/// As the PdmPtrArrayFields can hold several pointers to the same object, the returned vector can 
/// contain multiple pointers to the same field. 
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::referringPtrFields(std::vector<PdmFieldHandle*>& fieldsReferringToMe) const
{
    std::multiset<PdmFieldHandle*>::const_iterator it;

    for (it = m_referencingPtrFields.begin(); it != m_referencingPtrFields.end(); ++it)
    {
        fieldsReferringToMe.push_back(*it);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::objectsWithReferringPtrFields(std::vector<PdmObjectHandle*>& objects) const
{
    std::vector<caf::PdmFieldHandle*> parentFields;
    this->referringPtrFields(parentFields);
    size_t i;
    for (i = 0; i < parentFields.size(); i++)
    {
        objects.push_back(parentFields[i]->ownerObject());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::prepareForDelete()
{
    m_parentField = nullptr;

    for (size_t i = 0; i < m_capabilities.size(); ++i)
    {
        if (m_capabilities[i].second) delete m_capabilities[i].first;
    }

    // Set all guarded pointers pointing to this to NULL
    std::set<PdmObjectHandle**>::iterator it;
    for (it = m_pointersReferencingMe.begin(); it != m_pointersReferencingMe.end(); ++it)
    {
        (**it) = nullptr;
    }

    m_capabilities.clear();
    m_referencingPtrFields.clear();
    m_pointersReferencingMe.clear();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObjectHandle::addField(PdmFieldHandle* field, const QString& keyword)
{
    field->m_ownerObject = this;

    CAF_ASSERT(!keyword.isEmpty());
    CAF_ASSERT(this->findField(keyword) == nullptr);

    field->setKeyword(keyword);
    m_fields.push_back(field);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmFieldHandle* PdmObjectHandle::findField(const QString& keyword) const
{
    std::vector<PdmFieldHandle*> fields;
    this->fields(fields);

    for (size_t it = 0; it < fields.size(); it++)
    {
        PdmFieldHandle* field = fields[it];
        if (field->matchesKeyword(keyword))
        {
            return field;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmFieldHandle* PdmObjectHandle::parentField() const
{
    return m_parentField;
}

// These two functions can be used when PdmCore is used standalone without PdmUi/PdmXml
/*
PdmUiObjectHandle* PdmObjectHandle::uiCapability()
{
return NULL;
}

PdmXmlObjectHandle* PdmObjectHandle::xmlCapability()
{
return NULL;
}
*/


} // End namespace caf
