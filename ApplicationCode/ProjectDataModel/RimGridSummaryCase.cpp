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

    CAF_PDM_InitField(&m_userName, "UserName", QString("User Name"), "Case Name", "", "", "");
    m_userName.xmlCapability()->setIOReadable(false);
    m_userName.xmlCapability()->setIOWritable(false);
    m_userName.uiCapability()->setUiReadOnly(true);

    CAF_PDM_InitField(&m_summaryHeaderFilename, "SummaryHeaderFile", QString("Summary Header File"), "Summary File", "", "", "");
    m_summaryHeaderFilename.xmlCapability()->setIOReadable(false);
    m_summaryHeaderFilename.xmlCapability()->setIOWritable(false);
    m_summaryHeaderFilename.uiCapability()->setUiReadOnly(true);
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

    updateUiNames();
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
void RimGridSummaryCase::initAfterRead()
{
    RimSummaryCase::updateOptionSensitivity();

    updateUiNames();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridSummaryCase::updateUiNames()
{
    m_summaryHeaderFilename = summaryHeaderFilename();

    m_userName = curveDisplayName() + " - " + baseName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridSummaryCase::baseName() const
{
    if (!m_eclipseCase()) return QString();

    QFileInfo gridFileInfo(m_eclipseCase()->gridFileName());

    return gridFileInfo.completeBaseName();
}

