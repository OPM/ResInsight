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

#include "Ric3dViewPickEventHandler.h"

//==================================================================================================
///
//==================================================================================================
class RicMeasurementPickEventHandler : public Ric3dViewPickEventHandler
{
public:
    static RicMeasurementPickEventHandler* instance();

    void registerAsPickEventHandler() override;
    void unregisterAsPickEventHandler() override;

    void enablePolyLineMode( bool polyLineModeEnabled );

protected:
    RicMeasurementPickEventHandler();
    bool handle3dPickEvent( const Ric3dPickEvent& eventObject ) override;
    void notifyUnregistered() override;

private:
    bool m_polyLineModeEnabled;
};
