/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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


class RimReservoirView;
class RimResultSlot;
class QString;
class RigCaseData;

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
	RiuResultTextBuilder(RimReservoirView* reservoirView, size_t gridIndex, size_t cellIndex, size_t timeStepIndex);
	void setPickInfo(cvf::Part* part, cvf::uint partFaceHit, cvf::Part* nncPart, cvf::uint nncFaceHit, cvf::Vec3d intersectionPoint);

    QString mainResultText();

	QString gridResultDetails();
	QString faultResultDetails();
	QString cellEdgeResultDetails();

private:
	QString gridResultText();
	QString nncResultText();

	void appendTextFromResultSlot(RigCaseData* eclipseCase, size_t gridIndex, size_t cellIndex, size_t timeStepIndex, RimResultSlot* resultSlot, QString* resultInfoText);

private:
    caf::PdmPointer<RimReservoirView> m_reservoirView;

	size_t m_gridIndex;
	size_t m_cellIndex;
	size_t m_timeStepIndex;

	cvf::StructGridInterface::FaceType m_face;

	cvf::Part* m_part;
	uint	   m_partFaceIndex;

	cvf::Part* m_nncPart;
	uint	   m_nncPartFaceIndex;

	cvf::Vec3d m_intersectionPoint;
};
