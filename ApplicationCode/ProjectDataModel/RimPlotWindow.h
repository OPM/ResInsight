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

#include "RimNameConfig.h"
#include "RimViewWindow.h"

#include "cafPdmField.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmObject.h"

class QKeyEvent;

class RimPlotWindow : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimPlotWindow();

    RimPlotWindow& operator=( RimPlotWindow&& rhs );

    virtual void    setDescription( const QString& description );
    QString         description() const;
    virtual QString fullPlotTitle() const;

    bool isPlotTitleVisible() const;
    void setPlotTitleVisible( bool visible );
    bool legendsVisible() const;
    void setLegendsVisible( bool doShow );
    bool legendsHorizontal() const;
    void setLegendsHorizontal( bool horizontal );
    int  legendFontSize() const;
    void setLegendFontSize( int fontSize );

    virtual void handleKeyPressEvent( QKeyEvent* keyEvent ) {}
    virtual void updateLayout() = 0;

protected:
    void                          fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                                                    const QVariant&            oldValue,
                                                    const QVariant&            newValue ) override;
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;
    caf::PdmFieldHandle*          userDescriptionField() override;

    virtual void uiOrderingForPlotLayout( caf::PdmUiOrdering& uiOrdering );
    virtual void updatePlotTitle() = 0;

protected:
    caf::PdmField<QString> m_description;
    caf::PdmField<bool>    m_showTitleInPlot;
    caf::PdmField<bool>    m_showPlotLegends;
    caf::PdmField<bool>    m_plotLegendsHorizontal;
    caf::PdmField<int>     m_legendFontSize;
};
