/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

#include "cvfArray.h"

class RimColorLegendItem;

namespace caf
{
class PdmUiEditorAttribute;
}

//==================================================================================================
///
///
//==================================================================================================
class RimColorLegend : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimColorLegend();
    ~RimColorLegend() override;

public:
    void    setColorLegendName( const QString& colorLegendName );
    QString colorLegendName();

    void setReadOnly( bool doReadOnly );
    void addReorderCapability();

    void                             appendColorLegendItem( RimColorLegendItem* colorLegendItem );
    std::vector<RimColorLegendItem*> colorLegendItems() const;

    cvf::Color3ubArray colorArray() const;
    caf::IconProvider  paletteIconProvider() const;

    void onColorLegendItemHasChanged();

public:
    caf::PdmFieldHandle* userDescriptionField() override;

    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName ) override;

private:
    void orderChanged( const caf::SignalEmitter* emitter );

private:
    caf::PdmField<QString>                       m_colorLegendName;
    caf::PdmChildArrayField<RimColorLegendItem*> m_colorLegendItems;
};
