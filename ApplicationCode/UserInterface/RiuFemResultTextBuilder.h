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

class RimGeoMechView;
class RimResultSlot;
class RigGeoMechCaseData;
class RimGeoMechResultSlot;

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
	RiuFemResultTextBuilder(RimGeoMechView* reservoirView, size_t gridIndex, size_t cellIndex, size_t timeStepIndex);
    void setFace(cvf::StructGridInterface::FaceType face);
    void setIntersectionPoint(cvf::Vec3d intersectionPoint);

    QString mainResultText();

	QString topologyText(QString itemSeparator);
	
private:
    void appendDetails(QString& text, const QString& details);

    QString gridResultDetails();

    QString closestNodeResultText(RimGeoMechResultSlot* resultSlot);

	void appendTextFromResultSlot(RigGeoMechCaseData* eclipseCase, size_t gridIndex, size_t cellIndex, size_t timeStepIndex, RimGeoMechResultSlot* resultSlot, QString* resultInfoText);

private:
    caf::PdmPointer<RimGeoMechView> m_reservoirView;

	size_t m_gridIndex;
	size_t m_cellIndex;
	size_t m_timeStepIndex;

	cvf::StructGridInterface::FaceType m_face;

	cvf::Vec3d m_intersectionPoint;
};
