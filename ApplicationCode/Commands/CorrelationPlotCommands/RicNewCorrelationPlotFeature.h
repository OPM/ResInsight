/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "cafCmdFeature.h"

#include <ctime>

class RimSummaryCaseCollection;

class CorrelationPlotParams
{
public:
    RimSummaryCaseCollection* ensemble;
    QString                   quantityName;
    QString                   ensembleParameter;
    std::time_t               timeStep;

    CorrelationPlotParams();
    CorrelationPlotParams( const CorrelationPlotParams& rhs ) = default;

    CorrelationPlotParams( RimSummaryCaseCollection* ensemble, const QString& quantityName, const std::time_t& timeStep );
    ~CorrelationPlotParams() = default;
};

Q_DECLARE_METATYPE( CorrelationPlotParams );

//==================================================================================================
///
//==================================================================================================
class RicNewCorrelationPlotFeature : public caf::CmdFeature
{
    CAF_CMD_HEADER_INIT;

protected:
    // Overrides
    bool isCommandEnabled() override;
    void onActionTriggered( bool isChecked ) override;
    void setupActionLook( QAction* actionToSetup ) override;
};
