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
#include "cafPdmFieldCvfColor.h"
#include "cafPdmObject.h"
#include "cafPdmProxyValueField.h"

namespace caf
{
class PdmUiEditorAttribute;
}

//==================================================================================================
///
///
//==================================================================================================
class RimColorLegendItem : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimColorLegendItem();
    ~RimColorLegendItem() override;

    void setValues( const QString& categoryName, int categoryValue, const cvf::Color3f& color );
    void setCategoryValue( int categoryValue );
    void setReadOnly( bool doReadOnly );

    const cvf::Color3f& color() const;
    const QString&      categoryName() const;
    int                 categoryValue() const;

    QString itemName() const;

public:
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ ) override;

    caf::PdmFieldHandle* userDescriptionField() override;

private:
    caf::PdmField<cvf::Color3f> m_color;
    caf::PdmField<int>          m_categoryValue;
    caf::PdmField<QString>      m_categoryName;

    caf::PdmProxyValueField<QString> m_nameProxy;
};
