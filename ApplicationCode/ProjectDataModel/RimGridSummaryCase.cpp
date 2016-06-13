/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimGridSummaryCase.h"
#include "RimEclipseCase.h"
#include <QFileInfo>
#include "RigSummaryCaseData.h"

//==================================================================================================
//
// 
//
//==================================================================================================


CAF_PDM_SOURCE_INIT(RimGridSummaryCase,"GridSummaryCase");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGridSummaryCase::RimGridSummaryCase()
{
    CAF_PDM_InitFieldNoDefault(&m_eclipseCase, "Associated3DCase", "Main View", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGridSummaryCase::~RimGridSummaryCase()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridSummaryCase::setAssociatedEclipseCase(RimEclipseCase* eclipseCase)
{
    m_eclipseCase = eclipseCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridSummaryCase::summaryHeaderFilename() const
{
    if (!m_eclipseCase()) return QString();

    QFileInfo gridFileInfo(m_eclipseCase()->gridFileName());

    QString possibleSumHeaderFileName = gridFileInfo.path() +"/"+ gridFileInfo.completeBaseName() + ".SMSPEC";

    return possibleSumHeaderFileName;
}

