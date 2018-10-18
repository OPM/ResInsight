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
#include "cafPdmProxyValueField.h"
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

protected:
    void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue) override;
    void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    void defineEditorAttribute(const caf::PdmFieldHandle* field,
                               QString                    uiConfigName,
                               caf::PdmUiEditorAttribute* attribute) override;

private:
    QString getStatusText() const;
    void    setStatusText(const QString& statusText);

private:
    caf::PdmChildField<RicHoloLensServerSettings*> m_serverSettings;
    caf::PdmField<bool>                            m_createSession;

    caf::PdmField<QString>           m_sessionName;
    caf::PdmField<QString>           m_sessionPinCode;
    caf::PdmProxyValueField<QString> m_statusTextProxy;

    QString m_statusText;
};
