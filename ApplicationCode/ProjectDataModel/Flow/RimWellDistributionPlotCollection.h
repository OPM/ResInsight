/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimMultiPlotWindow.h"
#include "RiaDefines.h"

#include "cafPdmPtrField.h"

#include <QPointer>

#include <array>

class RimEclipseResultCase;
class RimFlowDiagSolution;
class RigTofWellDistributionCalculator;

class QTextBrowser;
class QwtPlot;


//==================================================================================================
//
//
//
//==================================================================================================
class RimWellDistributionPlotCollection : public RimMultiPlotWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellDistributionPlotCollection();
    ~RimWellDistributionPlotCollection() override;

private:
    // RimPlotWindow overrides
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    virtual void                            fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    virtual void        onLoadDataAndUpdate() override;

private:
    virtual void        defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void                fixupDependentFieldsAfterCaseChange();
    void                applyPlotParametersToContainedPlots();

private:
    caf::PdmPtrField<RimEclipseResultCase*> m_case;
    caf::PdmField<int>                      m_timeStepIndex;
    caf::PdmField<QString>                  m_wellName;
    caf::PdmField<bool>                     m_groupSmallContributions;
    caf::PdmField<double>                   m_smallContributionsRelativeThreshold;
};
