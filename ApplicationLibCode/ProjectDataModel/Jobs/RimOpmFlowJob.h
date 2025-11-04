/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimGenericJob.h"

#include "cafPdmPtrField.h"

#include <string>

class RimEclipseCase;
class RimWellPath;
class RifOpmFlowDeckFile;
class RimEclipseCaseEnsemble;
class RimSummaryEnsemble;
class RimKeywordWconprod;
class RimKeywordWconinje;

//==================================================================================================
///
///
//==================================================================================================
class RimOpmFlowJob : public RimGenericJob
{
    CAF_PDM_HEADER_INIT;

public:
    enum class WellOpenType
    {
        OPEN_BY_POSITION,
        OPEN_AT_DATE
    };

    enum class DateAppendType
    {
        ADD_DAYS,
        ADD_MONTHS
    };

public:
    RimOpmFlowJob();
    ~RimOpmFlowJob() override;

    void setWorkingDirectory( QString workDir );
    void setEclipseCase( RimEclipseCase* eCase );
    void setInputDataFile( QString filename );
    void initAfterCopy();

    QString deckName();
    QString mainWorkingDirectory() const;

protected:
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          initAfterRead() override;

    void decodeProgress( const QString& logLine ) override;

    QStringList                command() override;
    std::map<QString, QString> environment() override;
    QString                    workingDirectory() const override;
    bool                       onPrepare() override;
    bool                       onRun() override;
    void                       onCompleted( bool success ) override;
    void                       onProgress( double percentageDone ) override;

    bool openDeckFile();
    void closeDeckFile();
    bool copyUnrstFileToWorkDir();

private:
    RimEclipseCase* findExistingCase( QString filename );
    QString         deckExtension() const;
    QString         baseDeckName() const;
    QString         restartDeckName() const;

    std::vector<QDateTime> datesInFileDeck();
    std::vector<QString>   wellgroupsInFileDeck();

    std::vector<QString>   dateStrings();
    std::vector<QDateTime> dateTimes();
    std::vector<QDateTime> addedDateTimes();

    int  mergeBasicWellSettings();
    int  mergeMswData( int mergePosition );
    void selectOpenWellPosition();
    void resetEnsembleRunId();

private:
    caf::PdmField<caf::FilePath> m_deckFileName;
    caf::PdmField<caf::FilePath> m_workDir;
    caf::PdmField<int>           m_openWellDeckPosition;

    caf::PdmField<bool> m_pauseBeforeRun;
    caf::PdmField<bool> m_addToEnsemble;
    caf::PdmField<int>  m_currentRunId;
    caf::PdmField<bool> m_useRestart;

    caf::PdmPtrField<RimWellPath*>            m_wellPath;
    caf::PdmPtrField<RimEclipseCase*>         m_eclipseCase;
    caf::PdmPtrField<RimEclipseCaseEnsemble*> m_gridEnsemble;
    caf::PdmPtrField<RimSummaryEnsemble*>     m_summaryEnsemble;

    caf::PdmField<int>                        m_openTimeStep;
    caf::PdmField<bool>                       m_endTimeStepEnabled;
    caf::PdmField<int>                        m_endTimeStep;
    caf::PdmField<bool>                       m_addNewWell;
    caf::PdmField<caf::AppEnum<WellOpenType>> m_wellOpenType;
    caf::PdmField<bool>                       m_includeMSWData;
    caf::PdmField<QString>                    m_wellGroupName;

    caf::PdmField<bool>                         m_appendNewDates;
    caf::PdmField<int>                          m_newDatesInterval;
    caf::PdmField<int>                          m_numberOfNewDates;
    caf::PdmField<caf::AppEnum<DateAppendType>> m_dateAppendType;

    caf::PdmChildField<RimKeywordWconprod*> m_wconprodKeyword;
    caf::PdmChildField<RimKeywordWconinje*> m_wconinjeKeyword;

    caf::PdmField<QString> m_wellOpenKeyword;

    QString                             m_deckName;
    std::unique_ptr<RifOpmFlowDeckFile> m_deckFile;
    bool                                m_fileDeckHasDates;
    bool                                m_fileDeckIsRestart;
    int                                 m_startStepForProgress;
};
