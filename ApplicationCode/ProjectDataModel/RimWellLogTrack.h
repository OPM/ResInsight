/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "cafPdmObject.h"
#include "cafPdmField.h"
#include "cafPdmChildArrayField.h"
#include "cvfCollection.h"
#include "cafPdmPtrField.h"

#include <QPointer>

#include <memory>
#include <vector>

class RigWellPath;
class RimCase;
class RimWellFlowRateCurve;
class RimWellLogCurve;
class RimWellPath;
class RiuPlotAnnotationTool;
class RiuWellLogTrack;

class QwtPlotCurve;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogTrack : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;
public:
    RimWellLogTrack();
    virtual ~RimWellLogTrack();

    enum TrajectoryType { WELL_PATH, SIMULATION_WELL };

    void setDescription(const QString& description);
    bool isVisible();
    void addCurve(RimWellLogCurve* curve);
    void insertCurve(RimWellLogCurve* curve, size_t index);
    void removeCurve(RimWellLogCurve* curve);
    size_t curveIndex(RimWellLogCurve* curve);
    size_t curveCount() { return curves.size(); }
    void setXAxisTitle(const QString& text);

    void recreateViewer();
    void detachAllCurves();

    void loadDataAndUpdate();

    void availableDepthRange(double* minimumDepth, double* maximumDepth);
    void updateXZoomAndParentPlotDepthZoom();
    void updateXZoom();

    RiuWellLogTrack* viewer();
    
    RimWellLogCurve* curveDefinitionFromCurve(const QwtPlotCurve* curve) const;

    void setLogarithmicScale(bool enable);

    std::vector<RimWellFlowRateCurve*> visibleStackedCurves();

    QString description();
    std::vector<RimWellLogCurve* > curvesVector();

protected:

    // Overridden PDM methods
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;

    void computeAndSetXRangeMinForLogarithmicScale();

    virtual caf::PdmFieldHandle* objectToggleField() override;
    virtual caf::PdmFieldHandle* userDescriptionField() override;
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    
    static void updateGeneratedSimulationWellpath(cvf::Collection<RigWellPath>* generatedSimulationWellPathBranches, const QString& simWellName, RimCase* rimCase);
    static void simWellOptionItems(QList<caf::PdmOptionItemInfo>* options, RimCase* eclCase);
    static void clearGeneratedSimWellPaths(cvf::Collection<RigWellPath>* generatedSimulationWellPathBranches);
    void updateFormationNamesOnPlot();
    void removeFormationNames();
    void updateAxisScaleEngine();

private:
    QString m_xAxisTitle;

    caf::PdmField<bool> m_show;
    caf::PdmField<QString> m_userName;
    caf::PdmChildArrayField<RimWellLogCurve*> curves;
    caf::PdmField<double> m_visibleXRangeMin;
    caf::PdmField<double> m_visibleXRangeMax;
    caf::PdmField<bool>   m_isAutoScaleXEnabled;
    caf::PdmField<bool>   m_isLogarithmicScaleEnabled;

    caf::PdmField<bool>                             m_showFormations;
    caf::PdmPtrField<RimCase*>                      m_case;
    caf::PdmField<caf::AppEnum<TrajectoryType> >    m_trajectoryType;
    caf::PdmPtrField<RimWellPath*>                  m_wellPath;
    caf::PdmField<QString>                          m_simWellName;
    caf::PdmField<int>                              m_branchIndex;

    cvf::Collection<RigWellPath>                    m_generatedSimulationWellPathBranches;


    QPointer<RiuWellLogTrack> m_wellLogTrackPlotWidget;
    
    std::unique_ptr<RiuPlotAnnotationTool> m_annotationTool;
};
