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

#include <QPointer>

class RiuWellLogPlot;
class RimWellLogPlotTrack;


//==================================================================================================
///  
///  
//==================================================================================================
class RimWellLogPlot : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum DepthTypeEnum
    {
        MEASURED_DEPTH,
        TRUE_VERTICAL_DEPTH
    };

public:
    RimWellLogPlot();
    virtual ~RimWellLogPlot();

    void setDescription(const QString& description);
    void updateViewerWidgetWindowTitle();

    void    addTrack(RimWellLogPlotTrack* track);
    size_t  trackCount() { return tracks.size();}
    void    removeTrack(RimWellLogPlotTrack* track);

    void loadDataAndUpdate();
    void updateTracks();

    RiuWellLogPlot* viewer();

    void zoomDepth(double zoomFactor, double zoomCenter);
    void panDepth(double panFactor);
    void setVisibleDepthRange(double minimumDepth, double maximumDepth);

    void updateAvailableDepthRange();
    void availableDepthRange(double* minimumDepth, double* maximumDepth) const;
    bool hasAvailableDepthRange() const;

    void visibleDepthRange(double* minimumDepth, double* maximumDepth) const;
    void updateAxisRanges();
    void setVisibleDepthRangeFromContents();

    DepthTypeEnum depthType() const;
    QString depthPlotTitle() const;

    virtual caf::PdmFieldHandle* userDescriptionField()  { return &m_userName; }

    caf::PdmField< std::vector<int> >   windowGeometry;


protected:

    // Overridden PDM methods
    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    virtual void setupBeforeSave();
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering);

private:
    void updateViewerWidget();
    void recreateTrackPlots();
    void detachAllCurves();
    void handleViewerDeletion();

    virtual caf::PdmFieldHandle* objectToggleField();



private:
    QPointer<RiuWellLogPlot> m_viewer;
    
    caf::PdmField<bool>                 m_showWindow;

    caf::PdmChildArrayField<RimWellLogPlotTrack*> tracks;
    
    caf::PdmField<QString>                          m_userName;
    caf::PdmField< caf::AppEnum< DepthTypeEnum > >  m_depthType;
    caf::PdmField<double>                           m_minimumVisibleDepth;
    caf::PdmField<double>                           m_maximumVisibleDepth;

    double m_depthRangeMinimum;
    double m_depthRangeMaximum;

    friend class RiuWellLogPlot;
};
