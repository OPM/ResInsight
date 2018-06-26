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

#include "RiuInterfaceToViewWindow.h"

#include "cafPdmPointer.h"

#include <QList>
#include <QPointer>
#include <QWidget>

#include <map>

class RimWellLogPlot;
class RiuWellLogTrack;

class QFocusEvent;
class QLabel;
class QScrollBar;
class QwtLegend;

//==================================================================================================
//
// RiuWellLogPlot
//
//==================================================================================================
class RiuWellLogPlot : public QWidget, public RiuInterfaceToViewWindow
{
    Q_OBJECT

public:
    RiuWellLogPlot(RimWellLogPlot* plotDefinition, QWidget* parent = nullptr);
    virtual ~RiuWellLogPlot();

    RimWellLogPlot*                 ownerPlotDefinition();
    virtual RimViewWindow*          ownerViewWindow() const override;

    void                            addTrackPlot(RiuWellLogTrack* trackPlot);
    void                            insertTrackPlot(RiuWellLogTrack* trackPlot, size_t index);
    void                            removeTrackPlot(RiuWellLogTrack* trackPlot);

    void                            setDepthZoomAndReplot(double minDepth, double maxDepth);
    void                            setPlotTitle(const QString& plotTitle);

public slots:
    void                            updateChildrenLayout();

protected:
    virtual void                    resizeEvent(QResizeEvent *event);
    virtual void                    showEvent(QShowEvent *);
    virtual void                    changeEvent(QEvent *);
    virtual QSize                   sizeHint() const override;
    virtual void                    contextMenuEvent(QContextMenuEvent *) override;

private:
    void                            updateScrollBar(double minDepth, double maxDepth);
    void                            modifyWidthOfContainingMdiWindow(int widthChange);
    std::map<int, int>              calculateTrackWidths(int frameWidth);
    void                            placeChildWidgets(int frameHeight, int frameWidth);
    void                            positionTitle(int frameWidth);

private slots:
    void                            slotSetMinDepth(int value);
    void                            scheduleUpdateChildrenLayout();

private:
    QLabel*                           m_plotTitle;
    QScrollBar*                       m_scrollBar;
    QList<QPointer<QwtLegend> >       m_legends;
    QList<QPointer<RiuWellLogTrack> > m_trackPlots;
    caf::PdmPointer<RimWellLogPlot>   m_plotDefinition;
    QTimer*                           m_scheduleUpdateChildrenLayoutTimer;
};

