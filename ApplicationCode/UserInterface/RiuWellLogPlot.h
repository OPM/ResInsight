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

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
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
    RiuWellLogPlot( RimWellLogPlot* plotDefinition, QWidget* parent = nullptr );

    ~RiuWellLogPlot() override;

    RimWellLogPlot* ownerPlotDefinition();
    RimViewWindow*  ownerViewWindow() const override;

    void addTrackPlot( RiuWellLogTrack* trackPlot );
    void insertTrackPlot( RiuWellLogTrack* trackPlot, size_t index );
    void removeTrackPlot( RiuWellLogTrack* trackPlot );

    void          setDepthZoomAndReplot( double minDepth, double maxDepth );
    void          setPlotTitle( const QString& plotTitle );
    virtual QSize preferredSize() const;

    void setTitleVisible( bool visible );

public slots:
    void updateChildrenLayout();

protected:
    void showEvent( QShowEvent* ) override;
    void changeEvent( QEvent* event ) override;
    void contextMenuEvent( QContextMenuEvent* ) override;
    void keyPressEvent( QKeyEvent* keyEvent ) override;
    void resizeEvent( QResizeEvent* event ) override;

    QSize sizeHint() const override;

    QLabel* createTitleLabel() const;

private:
    void updateScrollBar( double minDepth, double maxDepth );
    void alignCanvasTops();
    void reinsertTracks();
private slots:
    void slotSetMinDepth( int value );

protected:
    QPointer<QVBoxLayout>            m_layout;
    QPointer<QHBoxLayout>            m_plotLayout;
    QPointer<QFrame>                 m_plotFrame;
    QPointer<QGridLayout>            m_trackLayout;
    QPointer<QLabel>                 m_plotTitle;
    QPointer<QVBoxLayout>            m_scrollBarLayout;
    QScrollBar*                      m_scrollBar;
    QList<QPointer<QwtLegend>>       m_legends;
    QList<QPointer<RiuWellLogTrack>> m_trackPlots;
    caf::PdmPointer<RimWellLogPlot>  m_plotDefinition;
};
