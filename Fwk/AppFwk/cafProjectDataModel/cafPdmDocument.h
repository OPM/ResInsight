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
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPointer.h"

namespace caf 
{

//==================================================================================================
/// The PdmObjectGroup serves as a container of unknown PdmObjects, and is inherited by
/// PdmDocument. Can be used to create sub assemblies.
/// This class should possibly be merged with PdmDocument. It is not clear whether it really has 
/// a reusable value on its own.
//==================================================================================================
class PdmObjectGroup : public PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    PdmObjectGroup();
    ~PdmObjectGroup();

    PdmPointersField<PdmObject*> objects;

    void                         deleteObjects();
    void                         removeNullPtrs();
    void                         addObject(PdmObject * obj);

    static void                  initAfterReadTraversal(PdmObject * root);

    template <typename T>
    void objectsByType(std::vector<PdmPointer<T> >* typedObjects ) const
    {
        if (!typedObjects) return;
        size_t it;
        for (it = 0; it != objects.size(); ++it)
        {
            T* obj = dynamic_cast<T*>(objects[it]);
            if (obj) typedObjects->push_back(obj);
        }
    }

    template <typename T>
    void createCopyByType(std::vector<PdmPointer<T> >* copyOfTypedObjects) const
    {
        std::vector<PdmPointer<T> > sourceTypedObjects;
        objectsByType(&sourceTypedObjects);

        QString encodedXml;
        {
            // Write original objects to XML file
            PdmObjectGroup typedObjectGroup;
            for (size_t i = 0; i < sourceTypedObjects.size(); i++)
            {
                typedObjectGroup.addObject(sourceTypedObjects[i]);
            }

            QXmlStreamWriter xmlStream(&encodedXml);
            xmlStream.setAutoFormatting(true);

            typedObjectGroup.writeFields(xmlStream);
        }

        // Read back XML into object group, factory methods will be called that will create new objects
        PdmObjectGroup destinationObjectGroup;
        QXmlStreamReader xmlStream(encodedXml);
        destinationObjectGroup.readFields(xmlStream);
        
        for (size_t it = 0; it < destinationObjectGroup.objects.size(); it++)
        {
            T* obj = dynamic_cast<T*>(destinationObjectGroup.objects[it]);
            if (obj) copyOfTypedObjects->push_back(obj);
        }
    }
};

//==================================================================================================
/// The PdmDocument class is the main class to do file based IO, 
/// and is also supposed to act as the overall container of the objects read.
//==================================================================================================
class PdmDocument: public PdmObjectGroup
{
    CAF_PDM_HEADER_INIT;
 public:
    PdmDocument();

    PdmField<QString>   fileName;

    void                readFile();
    void                writeFile();

    void                readFile(QIODevice* device);
    void                writeFile(QIODevice* device);

private:
    static void         setupBeforeSaveTraversal(PdmObject * root);
};



} // End of namespace caf


