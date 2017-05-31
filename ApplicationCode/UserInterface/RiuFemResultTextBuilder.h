/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "cvfBase.h"
#include "cafPdmPointer.h"
#include "cvfStructGrid.h"

#include <QString>
#include <array>

class RigGeoMechCaseData;
class RimEclipseCellColors;
class RimGeoMechResultDefinition;
class RimGeoMechView;

namespace cvf {
    class Part;
}

//==================================================================================================
//
//
//==================================================================================================
class RiuFemResultTextBuilder
{
public:
    RiuFemResultTextBuilder(RimGeoMechView* reservoirView, int gridIndex, int cellIndex, int timeStepIndex);
    void setFace(int face);
    void setIntersectionPoint(cvf::Vec3d intersectionPoint);
    void setIntersectionTriangle(const std::array<cvf::Vec3f, 3>& triangle);

    QString mainResultText();

    QString geometrySelectionText(QString itemSeparator);
    
private:
    void appendDetails(QString& text, const QString& details);

    QString gridResultDetails();
    QString formationDetails();

    QString closestNodeResultText(RimGeoMechResultDefinition* resultDefinition);

    void appendTextFromResultColors(RigGeoMechCaseData* eclipseCase, int gridIndex, int cellIndex, int timeStepIndex, RimGeoMechResultDefinition* resultDefinition, QString* resultInfoText);

private:
    caf::PdmPointer<RimGeoMechView> m_reservoirView;

    int m_gridIndex;
    int m_cellIndex;
    int m_timeStepIndex;

    int m_face;
    bool m_isIntersectionTriangleSet; 
    std::array<cvf::Vec3f, 3> m_intersectionTriangle;

    cvf::Vec3d m_intersectionPoint;
};
