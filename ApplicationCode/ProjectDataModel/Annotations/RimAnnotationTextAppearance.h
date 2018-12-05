/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RiaFontCache.h"

#include "cafPdmObject.h"

#include "cafPdmField.h"
#include "cafAppEnum.h"

#include "cafPdmFieldCvfColor.h" 


//==================================================================================================
///
///
//==================================================================================================
class RimAnnotationTextAppearance : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    using FontSize = caf::AppEnum<RiaFontCache::FontSize>;

    RimAnnotationTextAppearance();

    void                setFontSize(FontSize size);
    void                setFontColor(const cvf::Color3f& newColor);
    void                setBackgroundColor(const cvf::Color3f& newColor);
    void                setAnchorLineColor(const cvf::Color3f& newColor);
    
    FontSize            fontSize() const;
    cvf::Color3f        fontColor() const;
    cvf::Color3f        backgroundColor() const;
    cvf::Color3f        anchorLineColor() const;

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;

private:
    caf::PdmField<FontSize>         m_fontSize;
    caf::PdmField<cvf::Color3f>     m_fontColor;
    caf::PdmField<cvf::Color3f>     m_backgroundColor;
    caf::PdmField<cvf::Color3f>     m_anchorLineColor;

};

