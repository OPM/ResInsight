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
#include "cvfRegGrid2DImportXml.h"
#include "cvfRegGrid2D.h"
#include "cvfXml.h"

namespace cvf {



//==================================================================================================
///
/// \class cvf::RegGrid2DImportXml
/// \ingroup RegGrid2D
///
/// 
///
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RegGrid2DImportXml::importFromXmlFile(const String& filename, RegGrid2D* regGrid)
{
    CVF_ASSERT(regGrid);

    ref<XmlDocument> doc = XmlDocument::create();
    if (!doc->loadFile(filename))
    {
        return false;
    }

    XmlElement* root = doc->getRootElement("");
    if (!root)
    {
        return false;
    }

    return importFromXml(*root, regGrid);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RegGrid2DImportXml::importFromXml(const XmlElement& root, RegGrid2D* regGrid)
{
    CVF_ASSERT(regGrid);

    const XmlElement* xmlRegGrid = root.firstChildElement("RegularGrid2D");
    if (!xmlRegGrid)
    {
        return false;
    }

    bool found = false;

    int gridPointCountI = xmlRegGrid->getAttributeInt("GridPointCountI", -1, &found);
    if (!found) return false;

    int gridPointCountJ = xmlRegGrid->getAttributeInt("GridPointCountJ", -1, &found);
    if (!found) return false;

    double spacingX = xmlRegGrid->getAttributeDouble("SpacingX", -1, &found);
    if (!found) return false;

    double spacingY = xmlRegGrid->getAttributeDouble("SpacingY", -1, &found);
    if (!found) return false;

    double offsetX = xmlRegGrid->getAttributeDouble("OffsetX", -1, &found);
    if (!found) return false;

    double offsetY = xmlRegGrid->getAttributeDouble("OffsetY", -1, &found);
    if (!found) return false;

    regGrid->setOffset(Vec2d(offsetX, offsetY));
    regGrid->setSpacing(Vec2d(spacingX, spacingY));
    regGrid->allocateGrid(gridPointCountI, gridPointCountJ);

    DoubleArray elevations;
    elevations.reserve(static_cast<size_t>(regGrid->gridPointCount()));

    const cvf::XmlElement* xmlElevations = xmlRegGrid->firstChildElement("Elevations");
    if (!xmlElevations) return false;

    String valueText = xmlElevations->valueText();

    std::vector<String> elevationStrings = valueText.split();
    size_t i;
    for (i = 0; i < elevationStrings.size(); i++)
    {
        elevations.add(elevationStrings[i].toDouble());
    }

    regGrid->setElevations(elevations);

    return true;
}

} // namespace cvf

