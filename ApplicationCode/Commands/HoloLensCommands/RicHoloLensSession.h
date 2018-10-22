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

#pragma once

#include <QString>

class RimGridView;

//==================================================================================================
///
//==================================================================================================
class RicHoloLensSession
{
public:
    RicHoloLensSession();

    static RicHoloLensSession* instance();

    bool createSession(const QString& serverUrl, const QString& sessionName, const QString& sessionPinCode);
    bool createDummyFileBackedSession();

    bool isSessionValid() const;

    void updateSessionDataFromView(RimGridView* activeView);
    void terminateSession();

    static void refreshToolbarState();

private:
    bool m_isSessionValid;
    bool m_isDummySession;
};
