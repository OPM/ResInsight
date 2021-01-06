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

#include "cafFontTools.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

namespace caf
{
class TitledOverlayFrame;
}

//==================================================================================================
///
///
//==================================================================================================
class RimLegendConfig : public caf::PdmObject, public caf::FontHolderInterface
{
    CAF_PDM_HEADER_INIT;

public:
    RimLegendConfig();
    ~RimLegendConfig() override;

    enum class RangeModeType
    {
        AUTOMATIC_ALLTIMESTEPS,
        AUTOMATIC_CURRENT_TIMESTEP,
        USER_DEFINED
    };

    typedef caf::AppEnum<RangeModeType> RangeModeEnum;

    virtual const caf::TitledOverlayFrame* titledOverlayFrame() const = 0;
    virtual caf::TitledOverlayFrame*       titledOverlayFrame()       = 0;

    virtual int fontSize() const override;

    void recreateLegend();

protected:
    virtual void onRecreateLegend() = 0;
};
