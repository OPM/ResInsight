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

#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RicHoloLensServerSettings;

//==================================================================================================
///
//==================================================================================================
class RicHoloLensCreateSessionUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicHoloLensCreateSessionUi();
    ~RicHoloLensCreateSessionUi() override;

    QString serverUrl() const;
    QString sessionName() const;
    QString sessionPinCode() const;

protected:
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    caf::PdmChildField<RicHoloLensServerSettings*> m_serverSettings;

    caf::PdmField<QString> m_sessionName;
    caf::PdmField<QString> m_sessionPinCode;
};
