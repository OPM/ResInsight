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

#include "RiaRftPltCurveDefinition.h"

#include "RimCase.h"
#include "RimPlotCurve.h"

#include "RifWellRftAddressQMetaType.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"
#include "cvfCollection.h"

#include <QDate>
#include <QMetaType>
#include <QPointer>
#include <map>
#include <set>
#include <utility>

class RimEclipseCase;
class RimEclipseResultCase;
class RimWellLogCurve;
class RimWellLogFileChannel;
class RimWellLogPlot;
class RimWellPath;
class RiuWellRftPlot;
class RigEclipseCaseData;

namespace cvf {
    class Color3f;
}

namespace caf {
    class PdmOptionItemInfo;
}


//==================================================================================================
///  
///  
//==================================================================================================
class RimWellRftPlot : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;

    static const std::set<QString> PRESSURE_DATA_NAMES;
    static const char PLOT_NAME_QFORMAT_STRING[];

public:
    RimWellRftPlot();
    virtual ~RimWellRftPlot();

    void                                            setDescription(const QString& description);
    QString                                         description() const;

    virtual QWidget*                                viewWidget() override;
    virtual void                                    zoomAll() override;

    RimWellLogPlot*                                 wellLogPlot() const;

    void                                            setSimWellOrWellPathName(const QString& currWellName);
    QString                                         simWellOrWellPathName() const;
    int                                             branchIndex() const;

    static const char*                              plotNameFormatString();

    void                                            applyInitialSelections();

protected:
    // Overridden PDM methods
    virtual caf::PdmFieldHandle*                    userDescriptionField() override { return &m_userName; }
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

    virtual QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;

    virtual QImage                                  snapshotWindowContent() override;


    virtual void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void                                    onLoadDataAndUpdate() override;

private:
    void                                            calculateValueOptionsForWells(QList<caf::PdmOptionItemInfo>& options);
    void                                            calculateValueOptionsForTimeSteps(QList<caf::PdmOptionItemInfo>& options);

    void                                            updateEditorsFromCurves();
    void                                            updateWidgetTitleWindowTitle();

    void                                            syncCurvesFromUiSelection();

    std::set<RiaRftPltCurveDefinition>              selectedCurveDefs() const;
    std::set<RiaRftPltCurveDefinition>              curveDefsFromCurves() const;

    void                                            updateCurvesInPlot(const std::set<RiaRftPltCurveDefinition>& allCurveDefs,
                                                                       const std::set<RiaRftPltCurveDefinition>& curveDefsToAdd,
                                                                       const std::set<RimWellLogCurve*>& curvesToDelete);
    bool                                            isOnlyGridSourcesSelected() const;
    bool                                            isAnySourceAddressSelected(const std::set<RifWellRftAddress>& addresses) const;
    std::vector<RifWellRftAddress>                  selectedSources() const;

    // RimViewWindow overrides

    virtual QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                                    deleteViewWidget() override; 

    void                                            applyCurveAppearance(RimWellLogCurve* newCurve);
    void                                            updateSelectedTimeStepsFromSelectedSources();

    void                                            updateFormationsOnPlot() const;
    QString                                         associatedSimWellName() const;

private:
    caf::PdmField<bool>                             m_showPlotTitle;
    caf::PdmField<QString>                          m_userName;

    caf::PdmField<QString>                          m_wellPathNameOrSimWellName;
    caf::PdmField<int>                              m_branchIndex;
    caf::PdmField<std::vector<RifWellRftAddress>>   m_selectedSources;
    
    caf::PdmField<std::vector<QDateTime>>           m_selectedTimeSteps;

    QPointer<RiuWellRftPlot>                        m_wellLogPlotWidget;

    caf::PdmChildField<RimWellLogPlot*>             m_wellLogPlot;

    std::map<QDateTime, std::set<RifWellRftAddress>> m_timeStepsToAddresses;

    bool m_selectedSourcesOrTimeStepsFieldsChanged;
    bool m_isOnLoad;
};
