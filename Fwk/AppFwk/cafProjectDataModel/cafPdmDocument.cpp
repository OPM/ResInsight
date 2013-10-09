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


#include "cafPdmDocument.h"

#include <QFile>

namespace caf
{

CAF_PDM_SOURCE_INIT(PdmObjectGroup, "PdmObjectGroup");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectGroup::PdmObjectGroup()
{
    CAF_PDM_InitObject("Object Group", "", "", "");
    CAF_PDM_InitFieldNoDefault(&objects, "PdmObjects","", "", "", "")
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmObjectGroup::~PdmObjectGroup()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObjectGroup::deleteObjects()
{
    size_t it;
    for (it = 0; it != objects.size(); ++it)
    {
        delete objects[it];
    }
    removeNullPtrs();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObjectGroup::removeNullPtrs()
{
    objects.removeChildObject(NULL);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObjectGroup::addObject(PdmObject * obj)
{
    objects.push_back(obj);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmObjectGroup::initAfterReadTraversal(PdmObject* object)
{
    if (object == NULL) return;

    std::vector<PdmFieldHandle*> fields;
    object->fields(fields);

    std::vector<PdmObject*> children;
    size_t fIdx;
    for (fIdx = 0; fIdx < fields.size(); ++fIdx)
    {
        if (fields[fIdx]) fields[fIdx]->childObjects(&children);
    }

    size_t cIdx;
    for (cIdx = 0; cIdx < children.size(); ++cIdx)
    {
        PdmObjectGroup::initAfterReadTraversal(children[cIdx]);
    }

    object->initAfterRead();
}



CAF_PDM_SOURCE_INIT(PdmDocument, "PdmDocument");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmDocument::PdmDocument()
{
    CAF_PDM_InitObject("File", "", "", "");
    CAF_PDM_InitField(&fileName, "DocumentFileName", QString(""), "File Name", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmDocument::readFile()
{
    QFile xmlFile(fileName);
    if (!xmlFile.open(QIODevice::ReadOnly ))
        return;

    readFile(&xmlFile);
 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmDocument::readFile(QIODevice* xmlFile)
{
    QXmlStreamReader xmlStream(xmlFile);

    while (!xmlStream.atEnd())
    {
        xmlStream.readNext();
        if (xmlStream.isStartElement())
        {
            if (xmlStream.name() != classKeyword())
            {
                // Error: This is not a Ceetron Pdm based xml document
                return;
            }
            readFields(xmlStream);
        }
    }

    // Ask all objects to initialize and set up internal datastructure and pointers 
    // after everything is read from file

    PdmDocument::initAfterReadTraversal(this);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmDocument::writeFile()
{
    QFile xmlFile(fileName);
    if (!xmlFile.open(QIODevice::WriteOnly ))
        return;

    writeFile(&xmlFile);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmDocument::writeFile(QIODevice* xmlFile)
{
    // Ask all objects to make them ready to write themselves to file
    PdmDocument::setupBeforeSaveTraversal(this);

    QXmlStreamWriter xmlStream(xmlFile);
    xmlStream.setAutoFormatting(true);

    xmlStream.writeStartDocument();
    QString className = classKeyword(); 

    xmlStream.writeStartElement("", className);
    writeFields(xmlStream);
    xmlStream.writeEndElement();  

    xmlStream.writeEndDocument();
}


void PdmDocument::setupBeforeSaveTraversal(PdmObject * object)
{
    if (object == NULL) return;

    std::vector<PdmFieldHandle*> fields;
    object->fields(fields);

    std::vector<PdmObject*> children;
    size_t fIdx;
    for (fIdx = 0; fIdx < fields.size(); ++fIdx)
    {
        if (fields[fIdx]) fields[fIdx]->childObjects(&children);
    }

    size_t cIdx;
    for (cIdx = 0; cIdx < children.size(); ++cIdx)
    {
        PdmDocument::setupBeforeSaveTraversal(children[cIdx]);
    }

    object->setupBeforeSave();
}

} //End of namespace caf

