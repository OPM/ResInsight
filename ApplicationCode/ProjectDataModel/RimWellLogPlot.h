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

#include "cafAppEnum.h"
#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "RiaDefines.h"
#include "RimViewWindow.h"
#include "RimWellLogPlotNameConfig.h"

#include <QPointer>

class RimWellLogCurveCommonDataSource;
class RiuWellLogPlot;
class RimWellLogTrack;
class RimWellRftPlot;
class RimWellPltPlot;
class QKeyEvent;

//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogPlot : public RimViewWindow, public RimNameConfigHolderInterface
{
    CAF_PDM_HEADER_INIT;

public:
    enum DepthTypeEnum
    {
        MEASURED_DEPTH,
        TRUE_VERTICAL_DEPTH,
        PSEUDO_LENGTH,
        CONNECTION_NUMBER
    };

    enum AxisGridVisibility { AXIS_GRID_NONE, AXIS_GRID_MAJOR, AXIS_GRID_MAJOR_AND_MINOR };

    typedef caf::AppEnum<AxisGridVisibility> AxisGridEnum;

public:
    RimWellLogPlot();
    ~RimWellLogPlot() override;

    QWidget*                                        createPlotWidget();
    QWidget*                                viewWidget() override;

    void                                            setDescription(const QString& description);
    QString                                         description() const;

    DepthTypeEnum                                   depthType() const;
    void                                            setDepthType(DepthTypeEnum depthType);

    RiaDefines::DepthUnitType                       depthUnit() const;
    void                                            setDepthUnit(RiaDefines::DepthUnitType depthUnit);

    QString                                         depthPlotTitle() const;
    void                                            enableDepthGridLines(AxisGridVisibility gridVisibility);
    AxisGridVisibility                              depthGridLinesVisibility() const;

    bool                                            isPlotTitleVisible() const;
    void                                            setPlotTitleVisible(bool visible);    
    bool                                            areTrackLegendsVisible() const;
    void                                            setTrackLegendsVisible(bool doShow);
    bool                                            areTrackLegendsHorizontal() const;
    void                                            setTrackLegendsHorizontal(bool horizontal);

    void                                            addTrack(RimWellLogTrack* track);
    void                                            insertTrack(RimWellLogTrack* track, size_t index);
    size_t                                          trackCount() { return m_tracks.size();}

    void                                            removeTrack(RimWellLogTrack* track);
    size_t                                          trackIndex(const RimWellLogTrack* track) const;
    RimWellLogTrack*                                trackByIndex(size_t index);
    size_t                                          firstVisibleTrackIndex() const;

    void                                            updateTracks(bool autoScaleXAxis = false);
    void                                            updateTrackNames();

    void                                            updateDepthZoom();
    void                                            setDepthZoomByFactorAndCenter(double zoomFactor, double zoomCenter);
    void                                            panDepth(double panFactor);
    void                                            setDepthZoomMinMax(double minimumDepth, double maximumDepth);
    void                                            depthZoomMinMax(double* minimumDepth, double* maximumDepth) const;

    void                                            calculateAvailableDepthRange();
    void                                            availableDepthRange(double* minimumDepth, double* maximumDepth) const;
    bool                                            hasAvailableDepthRange() const;

    void                                    zoomAll() override;
    void                                            setDepthAutoZoom(bool on);
    void                                            enableAllAutoNameTags(bool enable);

    QString                                         asciiDataForPlotExport() const;

    RimWellRftPlot*                                 rftPlot() const;
    bool                                            isRftPlotChild() const;
    RimWellPltPlot*                                 pltPlot() const;
    bool                                            isPltPlotChild() const;

    void                                            uiOrderingForDepthAxis(caf::PdmUiOrdering& uiOrdering);
    void                                            uiOrderingForPlotSettings(caf::PdmUiOrdering& uiOrdering);

    QString                                 createAutoName() const override;
    void                                            updateHolder() override;

    void                                            handleKeyPressEvent(QKeyEvent* keyEvent);
    RimWellLogCurveCommonDataSource*                commonDataSource() const;
protected:

    // Overridden PDM methods
    void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    caf::PdmFieldHandle*                    userDescriptionField() override;
    QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    void                                    onLoadDataAndUpdate() override;

    QImage                                  snapshotWindowContent() override;

    QWidget*                                createViewWidget(QWidget* mainWindowParent) override;
    void                                    deleteViewWidget() override;

    void                                    initAfterRead() override;

private:
    void                                            applyZoomAllDepths();
    void                                            applyDepthZoomFromVisibleDepth();
    void                                            recreateTrackPlots();
    void                                            detachAllCurves();

    void                                            updateDisabledDepthTypes();
    void                                            updatePlotTitle();

private:
    caf::PdmField<QString>                                   m_userName_OBSOLETE;
    caf::PdmChildField<RimWellLogCurveCommonDataSource*>     m_commonDataSource;
    caf::PdmChildArrayField<RimWellLogTrack*>                m_tracks;

    caf::PdmField< caf::AppEnum<DepthTypeEnum>>              m_depthType;
    caf::PdmField< caf::AppEnum<RiaDefines::DepthUnitType>>  m_depthUnit;
    std::set<RimWellLogPlot::DepthTypeEnum>                  m_disabledDepthTypes;
    caf::PdmField<double>                                    m_minVisibleDepth;
    caf::PdmField<double>                                    m_maxVisibleDepth;
    caf::PdmField<AxisGridEnum>                              m_depthAxisGridVisibility;
    caf::PdmField<bool>                                      m_isAutoScaleDepthEnabled;
    
    caf::PdmField<bool>                                      m_showTitleInPlot;
    caf::PdmField<bool>                                      m_showTrackLegends;
    caf::PdmField<bool>                                      m_trackLegendsHorizontal;

    caf::PdmChildField<RimWellLogPlotNameConfig*>            m_nameConfig;

    double m_minAvailableDepth;
    double m_maxAvailableDepth;

    friend class RiuWellLogPlot;
    QPointer<RiuWellLogPlot> m_viewer;
};
