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

#include "RigSummaryCaseData.h"
#include "RimEclipseCase.h"

#include <QFileInfo>

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

    CAF_PDM_InitFieldNoDefault(&m_eclipseGridFileName, "Associated3DCaseGridFileName", "Grid File Name", "", "", "");
    m_eclipseGridFileName.registerGetMethod(this, &RimGridSummaryCase::eclipseGridFileName);
    m_eclipseGridFileName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitFieldNoDefault(&m_eclipseSummaryFileName, "SummaryFileName", "Summary File Name", "", "", "");
    m_eclipseSummaryFileName.registerGetMethod(this, &RimGridSummaryCase::summaryHeaderFilename);
    m_eclipseSummaryFileName.uiCapability()->setUiReadOnly(true);
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
    this->updateAutoShortName();
    this->updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimGridSummaryCase::associatedEclipseCase()
{
    return m_eclipseCase();
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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridSummaryCase::caseName() const
{
    if (!m_eclipseCase()) return QString();

    return m_eclipseCase()->caseUserDescription();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridSummaryCase::eclipseGridFileName() const
{
    if (!m_eclipseCase()) return QString();

    return m_eclipseCase()->gridFileName();
}

