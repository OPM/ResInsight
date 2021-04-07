/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016  Statoil ASA
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

#include "RimSummaryCase.h"

#include "cafPdmField.h"
#include "cvfObject.h"

class RifReaderRftInterface;
class RifReaderEclipseRft;
class RifReaderEclipseSummary;
class RiaThreadSafeLogger;

//==================================================================================================
//
//
//
//==================================================================================================
class RimFileSummaryCase : public RimSummaryCase
{
    CAF_PDM_HEADER_INIT;

public:
    RimFileSummaryCase();
    ~RimFileSummaryCase() override;

    QString summaryHeaderFilename() const override;
    QString caseName() const override;
    void    updateFilePathsFromProjectPath( const QString& newProjectPath, const QString& oldProjectPath ) override;

    void                       createSummaryReaderInterfaceThreadSafe( RiaThreadSafeLogger* threadSafeLogger );
    void                       createSummaryReaderInterface() override;
    void                       createRftReaderInterface() override;
    RifSummaryReaderInterface* summaryReader() override;
    RifReaderRftInterface*     rftReader() override;

    void setIncludeRestartFiles( bool includeRestartFiles );

    static RifReaderEclipseSummary* findRelatedFilesAndCreateReader( const QString&       headerFileName,
                                                                     bool                 includeRestartFiles,
                                                                     RiaThreadSafeLogger* threadSafeLogger );

    static RifReaderEclipseRft* findRftDataAndCreateReader( const QString& headerFileName );

private:
    cvf::ref<RifReaderEclipseSummary> m_summaryFileReader;
    cvf::ref<RifReaderEclipseRft>     m_summaryEclipseRftReader;
    caf::PdmField<bool>               m_includeRestartFiles;
};
