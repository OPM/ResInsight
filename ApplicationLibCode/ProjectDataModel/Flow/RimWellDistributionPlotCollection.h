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

#include "RiaDefines.h"
#include "RimPlotWindow.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QPointer>

#include <array>

class RimEclipseResultCase;
class RimFlowDiagSolution;
class RimPlot;
class RigTofWellDistributionCalculator;
class RiuMultiPlotPage;

class QTextBrowser;
class QwtPlot;

//==================================================================================================
//
//
//
//==================================================================================================
class RimWellDistributionPlotCollection : public RimPlotWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellDistributionPlotCollection();
    ~RimWellDistributionPlotCollection() override;

    void setData( RimEclipseResultCase* eclipseCase, QString wellName, int timeStepIndex );

    QWidget* viewWidget() override;
    QString  description() const override;
    QImage   snapshotWindowContent() override;
    void     zoomAll() override;

    caf::PdmFieldHandle* userDescriptionField() override;

private:
    // RimPlotWindow overrides
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;

    void     onLoadDataAndUpdate() override;
    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     deleteViewWidget() override;

    void doRenderWindowContent( QPaintDevice* paintDevice ) override;

private:
    void addPlot( RimPlot* plot );
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;
    void fixupDependentFieldsAfterCaseChange();
    void applyPlotParametersToContainedPlots();
    void updatePlots();
    void cleanupBeforeClose();
    void recreatePlotWidgets();

private:
    caf::PdmPtrField<RimEclipseResultCase*> m_case;
    caf::PdmField<int>                      m_timeStepIndex;
    caf::PdmField<QString>                  m_wellName;
    caf::PdmField<bool>                     m_groupSmallContributions;
    caf::PdmField<double>                   m_smallContributionsRelativeThreshold;
    caf::PdmField<double>                   m_maximumTof;

    caf::PdmField<QString>            m_plotWindowTitle;
    caf::PdmChildArrayField<RimPlot*> m_plots;

    caf::PdmField<bool> m_showOil;
    caf::PdmField<bool> m_showGas;
    caf::PdmField<bool> m_showWater;

    QPointer<RiuMultiPlotPage> m_viewer;
};
