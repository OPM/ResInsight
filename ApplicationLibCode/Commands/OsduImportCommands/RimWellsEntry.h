/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "cafAppEnum.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"

class RimWellPathEntry : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum WellTypeEnum
    {
        WELL_SURVEY,
        WELL_PLAN,
        WELL_ALL
    };

public:
    RimWellPathEntry();

    static RimWellPathEntry* createWellPathEntry( const QString& name,
                                                  const QString& surveyType,
                                                  const QString& requestUrl,
                                                  const QString& absolutePath,
                                                  WellTypeEnum   wellType );

    caf::PdmFieldHandle* userDescriptionField() override;
    caf::PdmFieldHandle* objectToggleField() override;

    caf::PdmField<QString> name;
    caf::PdmField<bool>    selected;

    caf::PdmField<caf::AppEnum<WellTypeEnum>> wellPathType;
    caf::PdmField<QString>                    surveyType;
    caf::PdmField<QString>                    requestUrl;

    bool                   isWellPathValid();
    caf::PdmField<QString> wellPathFilePath; // Location of the well path response

private:
    static QString undefinedWellPathLocation();
};
