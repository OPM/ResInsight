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

public:
    RimOpmFlowJob();
    ~RimOpmFlowJob() override;

    void setWorkingDirectory( QString workDir );
    void setEclipseCase( RimEclipseCase* eCase );
    void setInputDataFile( QString filename );

    QString deckName();

protected:
    void defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute ) override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          initAfterRead() override;

    QString     title() override;
    QStringList command() override;
    QString     workingDirectory() const override;
    bool        onPrepare() override;
    bool        onRun() override;
    void        onCompleted( bool success ) override;

    bool openDeckFile();
    bool copyUnrstFileToWorkDir();

private:
    RimEclipseCase* findExistingCase( QString filename );
    QString         deckExtension() const;
    QString         wellTempFile( int timeStep = -1, bool includeMSW = false, bool includeLGR = false ) const;
    QString         openWellTempFile() const;
    QString         baseDeckName() const;
    QString         restartDeckName() const;

    static QString readFileContent( QString filename );

    void        exportBasicWellSettings();
    std::string exportMswWellSettings( QDateTime date, int timeStep );
    QString     generateBasicOpenWellText();
    void        selectOpenWellPosition();

private:
    caf::PdmField<caf::FilePath> m_deckFileName;
    caf::PdmField<caf::FilePath> m_workDir;
    caf::PdmField<bool>          m_runButton;
    caf::PdmField<bool>          m_openSelectButton;
    caf::PdmField<int>           m_openWellDeckPosition;

    caf::PdmField<bool> m_pauseBeforeRun;
    caf::PdmField<bool> m_addToEnsemble;
    caf::PdmField<int>  m_currentRunId;
    caf::PdmField<bool> m_resetRunIdButton;
    caf::PdmField<bool> m_useRestart;

    caf::PdmPtrField<RimWellPath*>            m_wellPath;
    caf::PdmPtrField<RimEclipseCase*>         m_eclipseCase;
    caf::PdmPtrField<RimEclipseCaseEnsemble*> m_gridEnsemble;
    caf::PdmPtrField<RimSummaryEnsemble*>     m_summaryEnsemble;
    caf::PdmField<int>                        m_openTimeStep;
    caf::PdmField<bool>                       m_addNewWell;
    caf::PdmField<caf::AppEnum<WellOpenType>> m_wellOpenType;
    caf::PdmField<bool>                       m_includeMSWData;

    caf::PdmField<QString> m_wellOpenKeyword;
    caf::PdmField<QString> m_wellOpenText;

    QString                             m_deckName;
    std::unique_ptr<RifOpmFlowDeckFile> m_deckFile;
    bool                                m_fileDeckHasDates;
};
