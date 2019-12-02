/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Statoil ASA
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

#include "RimSummaryCaseCollection.h"
#include "cafAppEnum.h"

#include <utility>

class RimRegularLegendConfig;
class RiuCvfOverlayItemWidget;

class QFrame;

class RimEnsembleParameterColorHandlerInterface
{
public:
    enum class ColorMode
    {
        SINGLE_COLOR,
        BY_ENSEMBLE_PARAM
    };
    using ColorModeEnum     = caf::AppEnum<ColorMode>;
    using NameParameterPair = EnsembleParameter::NameParameterPair;

public:
    virtual ColorMode               colorMode() const                                    = 0;
    virtual void                    setColorMode( ColorMode mode )                       = 0;
    virtual void                    setEnsembleParameter( const QString& parameterName ) = 0;
    virtual void                    updateEnsembleLegendItem()                           = 0;
    virtual RimRegularLegendConfig* legendConfig()                                       = 0;
    virtual QFrame*                 legendFrame() const                                  = 0;
};
