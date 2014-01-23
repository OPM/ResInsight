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


#include "cvfBase.h"
#include "cvfRegGrid2DExportXml.h"
#include "cvfRegGrid2D.h"
#include "cvfXml.h"

namespace cvf {


//==================================================================================================
///
/// \class cvf::RegGrid2DExportXml
/// \ingroup RegGrid2D
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RegGrid2DExportXml::exportToXml(const RegGrid2D& regGrid, uint precision, XmlElement* parent)
{
    if (!parent)
    {
        return;
    }

    XmlElement* xmlRegGrid = parent->addChildElement("RegularGrid2D");
    CVF_ASSERT(xmlRegGrid);

    xmlRegGrid->setAttributeInt("GridPointCountI", regGrid.gridPointCountI());
    xmlRegGrid->setAttributeInt("GridPointCountJ", regGrid.gridPointCountJ());

    xmlRegGrid->setAttributeDouble("SpacingX", regGrid.spacing().x());
    xmlRegGrid->setAttributeDouble("SpacingY", regGrid.spacing().y());

    xmlRegGrid->setAttributeDouble("OffsetX", regGrid.offset().x());
    xmlRegGrid->setAttributeDouble("OffsetY", regGrid.offset().y());

    XmlElement* xmlElevations = xmlRegGrid->addChildElement("Elevations");
    CVF_ASSERT(xmlElevations);

    String valueText;
    int j;
    for (j = 0; j < regGrid.gridPointCountJ(); j++)
    {
        int i;
        for (i = 0; i < regGrid.gridPointCountI(); i++)
        {
            valueText += String::number(regGrid.elevation(i, j), 'f', static_cast<int>(precision));
            valueText += " ";
        }
    }

    xmlElevations->setValueText(valueText);
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RegGrid2DExportXml::exportToXmlFile(const RegGrid2D& regGrid, uint precision, const String& filename)
{
    ref<XmlDocument> doc = XmlDocument::create();
    CVF_ASSERT(doc.notNull());

    XmlElement* root = doc->createRootElement("Root");
    if (!root)
    {
        return false;
    }

    exportToXml(regGrid, precision, root);
    
    return doc->saveFile(filename);
}

} // namespace cvf

