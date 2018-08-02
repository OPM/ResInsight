/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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
#include "cafPdmObject.h"

class RicExportCarfinUi;
class RicExportCompletionDataSettingsUi;
class RiuCreateMultipleFractionsUi;

//==================================================================================================
///
/// This class is used as a container for UI specific data that is not part of the project tree view
/// Example of use is to store export settings for complex export dialogs or settings for advanced
/// creation of multiple objects
///
/// The data in this object will be stored in the project file, as RimDialogData is a child object of
/// RimProject
///
//==================================================================================================
class RimDialogData : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimDialogData();

    RicExportCarfinUi* exportCarfin() const;
    QString            exportCarfinDataAsString() const;
    void               setExportCarfinDataFromString(const QString& data);

    RicExportCompletionDataSettingsUi* exportCompletionData() const;

    RiuCreateMultipleFractionsUi* multipleFractionsData() const;

private:
    caf::PdmChildField<RicExportCarfinUi*>                 m_exportCarfin;
    caf::PdmChildField<RicExportCompletionDataSettingsUi*> m_exportCompletionData;
    caf::PdmChildField<RiuCreateMultipleFractionsUi*>      m_multipleFractionsData;
};
