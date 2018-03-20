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

#include "RifReaderEclipseSummary.h"

#include "RimEclipseCase.h"
#include "RimFileSummaryCase.h"
#include "RimProject.h"

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
    CAF_PDM_InitFieldNoDefault(&m_eclipseCase, "Associated3DCase", "Eclipse Case", "", "", "");
    m_eclipseCase.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_cachedCaseName, "CachedCasename", "Case Name", "", "", "");
    m_cachedCaseName.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_eclipseGridFileName, "Associated3DCaseGridFileName", "Grid File Name", "", "", "");
    m_eclipseGridFileName.registerGetMethod(this, &RimGridSummaryCase::eclipseGridFileName);
    m_eclipseGridFileName.uiCapability()->setUiReadOnly(true);
    m_eclipseGridFileName.xmlCapability()->setIOWritable(false);

    CAF_PDM_InitField(&m_includeRestartFiles, "IncludeRestartFiles", false, "Include Restart Files", "", "", "");
    m_includeRestartFiles.uiCapability()->setUiHidden(true);
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
QString summaryHeaderFilenameFromEclipseCase(RimEclipseCase* eclCase)
{
    if (!eclCase) return QString();

    QFileInfo gridFileInfo(eclCase->gridFileName());

    QString possibleSumHeaderFileName = gridFileInfo.path() +"/"+ gridFileInfo.completeBaseName() + ".SMSPEC";

    return possibleSumHeaderFileName; 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString caseNameFromEclipseCase(RimEclipseCase* eclCase)
{
    if (!eclCase) return QString();

    return eclCase->caseUserDescription();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridSummaryCase::setAssociatedEclipseCase(RimEclipseCase* eclipseCase)
{
    m_eclipseCase = eclipseCase;
    m_summaryHeaderFilename = summaryHeaderFilenameFromEclipseCase(eclipseCase);
    m_cachedCaseName = caseNameFromEclipseCase(eclipseCase);

    this->updateAutoShortName();
    this->updateTreeItemName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimGridSummaryCase::associatedEclipseCase()
{
    if (!m_eclipseCase())
    {
        // Find a possible associated eclipse case

        RimProject* project;
        firstAncestorOrThisOfTypeAsserted(project);
        std::vector<RimCase*> allCases;
        project->allCases(allCases);
        for ( RimCase* someCase: allCases )
        {
            auto eclCase = dynamic_cast<RimEclipseCase*>(someCase);
            if ( eclCase )
            {
                QString sumHeaderFileName = summaryHeaderFilenameFromEclipseCase(eclCase);
                if ( sumHeaderFileName == m_summaryHeaderFilename )
                {
                    m_eclipseCase = eclCase;
                    this->updateAutoShortName();
                    this->updateTreeItemName();

                    break;
                }
            }
        }
    }

    return m_eclipseCase();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridSummaryCase::summaryHeaderFilename() const
{
    if (!m_eclipseCase()) return m_summaryHeaderFilename();

    return summaryHeaderFilenameFromEclipseCase(m_eclipseCase());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridSummaryCase::caseName() 
{
    if (m_eclipseCase()) m_cachedCaseName = caseNameFromEclipseCase(m_eclipseCase());

    return m_cachedCaseName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimGridSummaryCase::eclipseGridFileName() const
{
    if (!m_eclipseCase()) return QString();

    return m_eclipseCase()->gridFileName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridSummaryCase::updateFilePathsFromProjectPath(const QString & newProjectPath, const QString & oldProjectPath)
{
    // Shouldn't have to do anything
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridSummaryCase::createSummaryReaderInterface()
{
    m_summaryFileReader = RimFileSummaryCase::findRelatedFilesAndCreateReader(this->summaryHeaderFilename(), m_includeRestartFiles);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RifSummaryReaderInterface* RimGridSummaryCase::summaryReader()
{
    return m_summaryFileReader.p();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGridSummaryCase::setIncludeRestartFiles(bool includeRestartFiles)
{
    m_includeRestartFiles = includeRestartFiles;
}

