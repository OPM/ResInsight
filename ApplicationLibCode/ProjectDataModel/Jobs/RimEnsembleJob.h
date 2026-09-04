/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

class RimReservoirGridEnsemble;
class RimEclipseCase;
class RimOpmFlowJob;
class RimOpmFlowJobSettings;
class RimJobWellSettings;

//==================================================================================================
///
//==================================================================================================
class RimEnsembleJob : public RimGenericJob
{
    CAF_PDM_HEADER_INIT;

public:
    RimEnsembleJob();
    ~RimEnsembleJob() override;

    void setEnsemble( RimReservoirGridEnsemble* ensemble );

    bool              execute() override;
    bool              stop() override;
    double            percentageDone() const override;
    const QStringList jobLog() const override;
    bool              matchesKeyValue( const QString& key, const QString& value ) const override;

    bool setFinished( bool runOk ) override;
    void setStarted() override;

protected:
    struct RealizationInfo
    {
        RimEclipseCase* inputCase;
        std::string     realizationInputDeckName;
        std::string     realizationOutputDir;
        std::string     outputDeckName;
    };

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;
    void                          defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    std::vector<RealizationInfo> getSelectedRealizations() const;
    std::string                  outputIteration() const;

private:
    std::vector<std::string> getSelectedRealizationFileNames() const;

private:
    caf::PdmPtrField<RimReservoirGridEnsemble*> m_ensemble;
    caf::PdmPtrArrayField<RimEclipseCase*>      m_selectedRealizations;
    caf::PdmField<int>                          m_outputIterationNumber;
    caf::PdmChildArrayField<RimOpmFlowJob*>     m_subJobs;
    caf::PdmChildField<RimOpmFlowJobSettings*>  m_jobSettings;
    caf::PdmChildField<RimJobWellSettings*>     m_jobWellSettings;
};
