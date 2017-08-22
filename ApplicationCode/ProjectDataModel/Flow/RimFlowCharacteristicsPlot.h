/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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


#include "RimViewWindow.h"

#include "RigFlowDiagResults.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QPointer>

class RimFlowDiagSolution;
class RimEclipseResultCase;

class RiuFlowCharacteristicsPlot;

namespace caf {
    class PdmOptionItemInfo;
}

namespace cvf {
    class Color3f;
}


//==================================================================================================
///  
///  
//==================================================================================================
class RimFlowCharacteristicsPlot : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimFlowCharacteristicsPlot();
    virtual ~RimFlowCharacteristicsPlot();

    void                                            setFromFlowSolution(RimFlowDiagSolution* flowSolution);
    void                                            updateCurrentTimeStep();

    // RimViewWindow overrides

    virtual QWidget*                                viewWidget() override;
    virtual void                                    zoomAll() override;
    virtual QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                                    deleteViewWidget() override; 
    virtual void                                    loadDataAndUpdate() override;

    enum TimeSelectionType 
    {
        ALL_AVAILABLE,
        SELECTED,
    };

protected:
    // RimViewWindow overrides

    virtual QImage                                  snapshotWindowContent() override;

    // Overridden PDM methods
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    virtual void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                                    defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute );

private:


    caf::PdmPtrField<RimEclipseResultCase*>         m_case;
    caf::PdmPtrField<RimFlowDiagSolution*>          m_flowDiagSolution;
    caf::PdmField<caf::AppEnum<TimeSelectionType> > m_timeStepSelectionType;
    caf::PdmField<std::vector<int> >                m_selectedTimeSteps;
    caf::PdmField<std::vector<int> >                m_selectedTimeStepsUi;
    caf::PdmField<bool>                             m_applyTimeSteps;
    caf::PdmField<bool>                             m_showLegend;
    caf::PdmField<double>                           m_maxPvFraction;

    caf::PdmField<RigFlowDiagResults::CellFilterEnum> m_cellFilter;
    caf::PdmField<QString>                          m_tracerFilter;
    caf::PdmField< std::vector<QString> >           m_selectedTracerNames;
    caf::PdmField<bool>                             m_showRegion;

    std::vector<int>                                m_currentlyPlottedTimeSteps;

    QPointer<RiuFlowCharacteristicsPlot>            m_flowCharPlotWidget;
};
