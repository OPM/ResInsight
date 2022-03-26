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
class RifOpmCommonEclipseSummary;
class RifEclipseSummaryAddress;
class RifMultipleSummaryReaders;

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

    void setSummaryData( const std::string& keyword, const std::string& unit, const std::vector<float>& values );
    void onProjectBeingSaved();

    static RifSummaryReaderInterface* findRelatedFilesAndCreateReader( const QString&       headerFileName,
                                                                       bool                 includeRestartFiles,
                                                                       RiaThreadSafeLogger* threadSafeLogger );

    static RifReaderEclipseRft* findRftDataAndCreateReader( const QString& headerFileName );

protected:
    void defineEditorAttribute( const caf::PdmFieldHandle* field,
                                QString                    uiConfigName,
                                caf::PdmUiEditorAttribute* attribute ) override;

private:
    void           openAndAttachAdditionalReader();
    QString        additionalSummaryDataFilePath() const;
    static QString createAdditionalSummaryFileName();

private:
    cvf::ref<RifSummaryReaderInterface> m_fileSummaryReader;
    cvf::ref<RifMultipleSummaryReaders> m_multiSummaryReader;
    cvf::ref<RifReaderEclipseRft>       m_summaryEclipseRftReader;
    caf::PdmField<bool>                 m_includeRestartFiles;

    caf::PdmField<caf::FilePath>         m_additionalSummaryFilePath;
    cvf::ref<RifOpmCommonEclipseSummary> m_additionalSummaryFileReader;
};
