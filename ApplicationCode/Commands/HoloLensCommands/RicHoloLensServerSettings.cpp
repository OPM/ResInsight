/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "RicHoloLensServerSettings.h"


CAF_PDM_SOURCE_INIT(RicHoloLensServerSettings, "RicHoloLensServerSettings");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensServerSettings::RicHoloLensServerSettings()
{
    CAF_PDM_InitObject("HoloLens Server Settings", "", "", "");

    CAF_PDM_InitField(&m_serverAddress, "ServerAddress", QString(), "Server Address", "", "", "");
}

