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

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QPointer>

#include <vector>

class RimWellLogPlot;
class RiuNightchartsWidget;
class RiuWellAllocationPlot;

namespace caf
{
class PdmOptionItemInfo;
}

namespace cvf
{
class Color3f;
}

//==================================================================================================
///
///
//==================================================================================================
class RimTotalWellAllocationPlot : public RimViewWindow
{
    CAF_PDM_HEADER_INIT;

public:
    RimTotalWellAllocationPlot();
    ~RimTotalWellAllocationPlot() override;

    int id() const final;

    void    setDescription( const QString& description );
    QString description() const;
    QString totalAllocationAsText() const;

    void addSlice( const QString& name, const cvf::Color3f& color, float value );
    void clearSlices();
    // RimViewWindow overrides

    QWidget* viewWidget() override;
    void     zoomAll() override;
    QWidget* createViewWidget( QWidget* mainWindowParent ) override;
    void     deleteViewWidget() override;

protected:
    // RimViewWindow overrides

    void   onLoadDataAndUpdate() override;
    QImage snapshotWindowContent() override;

    // Overridden PDM methods
    caf::PdmFieldHandle* userDescriptionField() override
    {
        return &m_userName;
    }
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField,
                           const QVariant&            oldValue,
                           const QVariant&            newValue ) override;

private:
    void assignIdIfNecessary() final;

private:
    caf::PdmField<bool>    m_showPlotTitle;
    caf::PdmField<QString> m_userName;

    QPointer<RiuNightchartsWidget> m_wellTotalAllocationPlotWidget;

    std::vector<std::pair<QString, float>> m_sliceInfo;
};
