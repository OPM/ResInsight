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
#include "cafAppEnum.h"

#include "RiaDefines.h"
#include "RimViewWindow.h"

#include <QPointer>

class RiuWellLogPlot;
class RimWellLogTrack;
class RimWellRftPlot;
class RimWellPltPlot;


//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogPlot : public RimViewWindow
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




public:
    RimWellLogPlot();
    virtual ~RimWellLogPlot();

    void                                            setDescription(const QString& description);
    QString                                         description() const;

    DepthTypeEnum                                   depthType() const;
    void                                            setDepthType(DepthTypeEnum depthType);

    RiaDefines::DepthUnitType                       depthUnit() const;
    void                                            setDepthUnit(RiaDefines::DepthUnitType depthUnit);


    QString                                         depthPlotTitle() const;
    bool                                            isPlotTitleVisible() const;
    bool                                            areTrackLegendsVisible() const;
    void                                            setTrackLegendsVisible(bool doShow);
    bool                                            areTrackLegendsHorizontal() const;

    void                                            addTrack(RimWellLogTrack* track);
    void                                            insertTrack(RimWellLogTrack* track, size_t index);
    size_t                                          trackCount() { return m_tracks.size();}
    void                                            removeTrackByIndex(size_t index);

    void                                            removeTrack(RimWellLogTrack* track);
    size_t                                          trackIndex(RimWellLogTrack* track);
    void                                            moveTracks(RimWellLogTrack* insertAfterTrack, const std::vector<RimWellLogTrack*>& tracksToMove);

    RimWellLogTrack*                                trackByIndex(size_t index);

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

    virtual void                                    zoomAll() override;
    virtual QWidget*                                viewWidget() override;
    void                                            setDepthAutoZoom(bool on);


    QString                                         asciiDataForPlotExport() const;

    RimWellRftPlot*                                 rftPlot() const;
    bool                                            isRftPlotChild() const;
    RimWellPltPlot*                                 pltPlot() const;
    bool                                            isPltPlotChild() const;

    void                                            uiOrderingForVisibleDepthRange(caf::PdmUiOrdering& uiOrdering);
    void                                            uiOrderingForPlot(caf::PdmUiOrdering& uiOrdering);

protected:

    // Overridden PDM methods
    virtual void                                    fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    virtual void                                    defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual caf::PdmFieldHandle*                    userDescriptionField() override { return &m_userName; }
    virtual QList<caf::PdmOptionItemInfo>           calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;
    virtual void                                    onLoadDataAndUpdate() override;

    virtual QImage                                  snapshotWindowContent() override;


private:
    void                                            applyZoomAllDepths();
    void                                            applyDepthZoomFromVisibleDepth();
    void                                            recreateTrackPlots();
    void                                            detachAllCurves();

    void                                            updateDisabledDepthTypes();
    void                                            updatePlotTitle();
public: // Needed by RiuWellAllocation Plot
    // RimViewWindow overrides

    virtual QWidget*                                createViewWidget(QWidget* mainWindowParent) override; 
    virtual void                                    deleteViewWidget() override; 

private:
    caf::PdmField<QString>                          m_userName;
    
    caf::PdmField< caf::AppEnum< DepthTypeEnum > >              m_depthType;
    caf::PdmField< caf::AppEnum< RiaDefines::DepthUnitType > >  m_depthUnit;
    std::set<DepthTypeEnum>                         m_disabledDepthTypes;

    caf::PdmChildArrayField<RimWellLogTrack*>       m_tracks;

    caf::PdmField<double>                           m_minVisibleDepth;
    caf::PdmField<double>                           m_maxVisibleDepth;
    caf::PdmField<bool>                             m_isAutoScaleDepthEnabled;
    caf::PdmField<bool>                             m_showTitleInPlot;
    caf::PdmField<bool>                             m_showTrackLegends;
    caf::PdmField<bool>                             m_trackLegendsHorizontal;

    double                                          m_minAvailableDepth;
    double                                          m_maxAvailableDepth;

    friend class RiuWellLogPlot;
    QPointer<RiuWellLogPlot>                        m_viewer;

};
