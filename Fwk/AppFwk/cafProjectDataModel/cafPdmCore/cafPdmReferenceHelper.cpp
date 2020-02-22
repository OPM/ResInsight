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


#include "cafPdmReferenceHelper.h"

#include "cafAssert.h"
#include "cafPdmFieldHandle.h"

#include <QStringList>

#include <algorithm>


namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString rootIdentifierString()
{
    return "$ROOT$";
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString PdmReferenceHelper::referenceFromRootToObject(PdmObjectHandle* root, PdmObjectHandle* obj)
{
    if (obj == nullptr || root == nullptr) return QString();

    QStringList objectNames = referenceFromRootToObjectAsStringList(root, obj);
    
    QString completeReference = objectNames.join(" ");
    return completeReference;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString PdmReferenceHelper::referenceFromRootToField(PdmObjectHandle* root, PdmFieldHandle* field)
{
    if (field == nullptr || root == nullptr) return QString();

    PdmObjectHandle* owner = field->ownerObject();
    if (!owner) return QString(); // Should be assert ?

    QStringList refFromRootToField;

    refFromRootToField = referenceFromRootToObjectAsStringList(root, owner);

    refFromRootToField.push_front(field->keyword());

    QString completeReference = refFromRootToField.join(" ");
    return completeReference;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmReferenceHelper::objectFromReference(PdmObjectHandle* root, const QString& reference)
{
    QStringList decodedReference = reference.split(" ");

    return objectFromReferenceStringList(root, decodedReference);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmFieldHandle* PdmReferenceHelper::findField(PdmObjectHandle* object, const QString& fieldKeyword)
{
    if (object == nullptr) return nullptr;

    std::vector<PdmFieldHandle*> fields;
    object->fields(fields);

    for (size_t i = 0; i < fields.size(); i++)
    {
        if (fields[i]->keyword() == fieldKeyword)
        {
            return fields[i];
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QStringList PdmReferenceHelper::referenceFromRootToObjectAsStringList(PdmObjectHandle* root, PdmObjectHandle* obj)
{
    QStringList objectNames;
    
    if (obj != nullptr && root)
    {
        if (obj == root) return objectNames;

        PdmObjectHandle* currentObject = obj;

        bool continueParsing = true;
        while (continueParsing)
        {
            caf::PdmFieldHandle* parentField = currentObject->parentField();
            if (!parentField)
            {
                // Could not find a path from obj to root, obj and root are unrelated objects
                return QStringList();
            }

            std::vector<PdmObjectHandle*> childObjects;
            parentField->childObjects(&childObjects);

            if (childObjects.size() > 0)
            {
                int index = -1;

                for (size_t i = 0; i < childObjects.size(); i++)
                {
                    if (childObjects[i] == currentObject)
                    {
                        index = static_cast<int>(i);
                    }
                }

                objectNames.push_front(QString::number(index));
                objectNames.push_front(parentField->keyword());
            }
            else
            {
                continueParsing = false;
                continue;
            }

            PdmObjectHandle* ownerObject = parentField->ownerObject();
            if (!ownerObject)
            {
                // Could not find a path from obj to root, obj and root are unrelated objects
                return QStringList();
            }

            if (ownerObject == root)
            {
                continueParsing = false;
                continue;
            }

            currentObject = ownerObject;
        }
    }

    return objectNames;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmFieldHandle* PdmReferenceHelper::fieldFromReference(PdmObjectHandle* root, const QString& reference)
{
    QStringList decodedReference = reference.split(" ");
    if (decodedReference.size() == 0) return nullptr;

    QString fieldKeyword = decodedReference[0];
    decodedReference.pop_front();

    PdmObjectHandle* parentObject = objectFromReferenceStringList(root, decodedReference);
    return findField(parentObject, fieldKeyword);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmReferenceHelper::objectFromReferenceStringList(PdmObjectHandle* root, const QStringList& reference)
{
    if (!root) return nullptr;

    PdmObjectHandle* currentObject = root;

    int i = 0;
    while (i < reference.size())
    {
        QString fieldKeyword = reference.at(i++);

        PdmFieldHandle* fieldHandle = findField(currentObject, fieldKeyword);
        if (!fieldHandle)
        {
            return nullptr;
        }

        std::vector<PdmObjectHandle*> childObjects;
        fieldHandle->childObjects(&childObjects);

        if (childObjects.size() == 0)
        {
            return nullptr;
        }

        QString fieldIndex = reference.at(i++);
        bool conversionOk = true;
        int index = fieldIndex.toInt(&conversionOk);
        if (!conversionOk)
        {
            return nullptr;
        }

        if (index < 0 || index > ((int)childObjects.size()) - 1)
        {
            return nullptr;
        }

        currentObject = childObjects[index];
    }

    return currentObject;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<PdmObjectHandle*> findPathToObjectFromRoot(PdmObjectHandle* obj)
{
    std::vector<PdmObjectHandle*> objPath;
    PdmObjectHandle* currentObj = obj;
    while (currentObj)
    {
        objPath.push_back(currentObj);
        if (currentObj->parentField())
        {
            currentObj = currentObj->parentField()->ownerObject();
        }
        else
        {
            currentObj = nullptr;
        }
    }

    std::reverse(objPath.begin(), objPath.end());

    return objPath;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString PdmReferenceHelper::referenceFromFieldToObject(PdmFieldHandle* fromField, PdmObjectHandle* toObj)
{
    if (!fromField || !toObj) return "";

   PdmObjectHandle* fromObj = fromField->ownerObject();
   if (!fromObj) return "";

   std::vector<PdmObjectHandle*> fromObjPath = findPathToObjectFromRoot(fromObj);
   std::vector<PdmObjectHandle*> toObjPath = findPathToObjectFromRoot(toObj);

   // Make sure the objects actually have at least one common ancestor
   if (fromObjPath.front() != toObjPath.front()) return nullptr;

   bool anchestorIsEqual = true;
   size_t idxToLastCommonAnchestor = 0;
   while (anchestorIsEqual)
   {
        ++idxToLastCommonAnchestor;
        if (   idxToLastCommonAnchestor >= fromObjPath.size() 
            || idxToLastCommonAnchestor >= toObjPath.size()
            || fromObjPath[idxToLastCommonAnchestor] != toObjPath[idxToLastCommonAnchestor])
        {
            anchestorIsEqual = false;
            idxToLastCommonAnchestor -= 1;
        }
   }

   size_t levelCountToCommonAnchestor = (fromObjPath.size() - 1) - idxToLastCommonAnchestor;

   PdmObjectHandle* lastCommonAnchestor = fromObjPath[idxToLastCommonAnchestor];

   QStringList referenceList = referenceFromRootToObjectAsStringList(lastCommonAnchestor, toObj);
   
   if (idxToLastCommonAnchestor == 0)
   {
       referenceList.push_front(rootIdentifierString());
   }
   else
   {
       for (size_t i = 0; i < levelCountToCommonAnchestor; ++i)
       {
           referenceList.push_front("..");
       }
   }
 
   QString completeReference = referenceList.join(" ");

   return completeReference;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmReferenceHelper::objectFromFieldReference(PdmFieldHandle* fromField, const QString& reference)
{
    if (!fromField) return nullptr;
    if (reference.isEmpty()) return nullptr;
    if (reference.trimmed().isEmpty()) return nullptr;

    QStringList decodedReference = reference.split(QRegExp("\\s+"), QString::SkipEmptyParts);
    PdmObjectHandle* lastCommonAnchestor = fromField->ownerObject();
    CAF_ASSERT(lastCommonAnchestor);
 
    if (!decodedReference.empty() && decodedReference.front() == rootIdentifierString())
    {
        lastCommonAnchestor = findRoot(lastCommonAnchestor);
        decodedReference.pop_front();
    }
    else
    {    
        while (!decodedReference.empty() && decodedReference.front() == "..")
        {
            PdmFieldHandle* parentField = lastCommonAnchestor->parentField();
            if (!parentField)
            {
                // Error: Relative object reference has an invalid number of parent levels
                return nullptr;
            }

            lastCommonAnchestor = parentField->ownerObject();
            CAF_ASSERT(lastCommonAnchestor);
            decodedReference.pop_front();
        }
    }

    return objectFromReferenceStringList(lastCommonAnchestor, decodedReference);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmReferenceHelper::findRoot(PdmObjectHandle* obj)
{
    std::vector<PdmObjectHandle*> path = findPathToObjectFromRoot(obj);

    if (path.size()) return path[0];
    else return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectHandle* PdmReferenceHelper::findRoot(PdmFieldHandle* field)
{
    if (field)
    {
        PdmObjectHandle* ownerObject = field->ownerObject();
        return findRoot(ownerObject);
    }
    
    return nullptr;
}

} // end namespace caf
