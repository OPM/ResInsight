#include "cafPdmObject.h"

using namespace caf;

CAF_PDM_ABSTRACT_SOURCE_INIT(PdmObject, "PdmObject");


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject::PdmObject() : PdmObjectHandle(), PdmXmlObjectHandle(this, false), PdmUiObjectHandle(this, false)
{
    CAF_PDM_InitObject("Base PDM Object", "", "", "The Abstract Base Class for the Project Data Model");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObject::firstAncestorOrThisFromClassKeyword(
    const QString& classKeyword, PdmObject*& ancestor) const
{
    ancestor = nullptr;

    // Check if this matches the type
    if (this->inheritsClassWithKeyword(classKeyword))
    {
        ancestor = const_cast<PdmObject*>(this);
        return;
    }

    // Search parents for first type match

    PdmObject* parent = nullptr;
    PdmFieldHandle* parentField = this->parentField();
    if (parentField) parent = dynamic_cast<PdmObject*>(parentField->ownerObject());

    while (parent != nullptr)
    {
        if (parent->inheritsClassWithKeyword(classKeyword))
        {
            ancestor = parent;
            return;
        }
        // Get next level parent

        PdmFieldHandle* nextParentField = parent->parentField();
        if (nextParentField)
        {
            parent = dynamic_cast<PdmObject*>(nextParentField->ownerObject());
        }
        else
        {
            parent = nullptr;
        }
    }  
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObject::descendantsIncludingThisFromClassKeyword(
    const QString& classKeyword,
    std::vector<PdmObject*>& descendants) const
{
    if (this->inheritsClassWithKeyword(classKeyword))
    {
        descendants.push_back(const_cast<PdmObject*>(this));
    }

    std::vector<PdmFieldHandle*> fields;
    this->fields(fields);
    for (auto f : fields)
    {
        std::vector<PdmObjectHandle*> childObjects;
        f->childObjects(&childObjects);
        for (auto childObject : childObjects)
        {
            PdmObject* pdmObjectChild = dynamic_cast<PdmObject*>(childObject);
            if (pdmObjectChild)
            {
                pdmObjectChild->descendantsIncludingThisFromClassKeyword(classKeyword, descendants);
            }
        }
    }
} 

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmObject::childrenFromClassKeyword(
    const QString& classKeyword,
    std::vector<PdmObject*>& children) const
{
    std::vector<PdmFieldHandle*> fields;
    this->fields(fields);
    for (auto f : fields)
    {
        std::vector<PdmObjectHandle*> childObjects;
        f->childObjects(&childObjects);
        for (auto childObject : childObjects)
        {
            PdmObject* pdmObjectChild = dynamic_cast<PdmObject*>(childObject);
            if (pdmObjectChild && pdmObjectChild->matchesClassKeyword(classKeyword))
            {
                children.push_back(pdmObjectChild);
            }
        }
    }
}
