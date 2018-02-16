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


class RimEclipseView;
class RimEclipseCellColors;
class Rim2dIntersectionView;
class QString;
class RigEclipseCaseData;

namespace cvf {
    class Part;
}

//==================================================================================================
//
//
//==================================================================================================
class RiuResultTextBuilder
{
public:
    RiuResultTextBuilder(RimEclipseView* reservoirView, size_t gridIndex, size_t cellIndex, size_t timeStepIndex);
    void setFace(cvf::StructGridInterface::FaceType face);
    void setNncIndex(size_t nncIndex);
    void setIntersectionPoint(cvf::Vec3d intersectionPoint);
    void set2dIntersectionView(Rim2dIntersectionView* intersectionView);

    QString mainResultText();

    QString geometrySelectionText(QString itemSeparator);
    
private:
    void appendDetails(QString& text, const QString& details);

    QString gridResultDetails();
    QString faultResultDetails();
    QString formationDetails();
    QString cellEdgeResultDetails();
    QString nncDetails();

    QString gridResultText();
    QString faultResultText();
    QString nncResultText();
    QString wellResultText();

    QString cellResultText(RimEclipseCellColors* resultColors);

    void appendTextFromResultColors(RigEclipseCaseData* eclipseCase, size_t gridIndex, size_t cellIndex, size_t timeStepIndex, RimEclipseCellColors* resultColors, QString* resultInfoText);

private:
    caf::PdmPointer<RimEclipseView> m_reservoirView;
    caf::PdmPointer<Rim2dIntersectionView> m_2dIntersectionView;

    size_t m_gridIndex;
    size_t m_cellIndex;
    size_t m_timeStepIndex;

    cvf::StructGridInterface::FaceType m_face;

    size_t      m_nncIndex;

    cvf::Vec3d m_intersectionPoint;
};
