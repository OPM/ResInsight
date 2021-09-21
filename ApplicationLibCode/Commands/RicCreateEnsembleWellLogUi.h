/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RiaDefines.h"

#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

#include <QString>
#include <QStringList>

class RigEclipseCaseData;
class RimWellPath;

//==================================================================================================
///
//==================================================================================================
class RicCreateEnsembleWellLogUi : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    enum class WellPathSource
    {
        FILE,
        PROJECT_WELLS
    };

    RicCreateEnsembleWellLogUi();
    ~RicCreateEnsembleWellLogUi() override;
    const QStringList& tabNames() const;

    bool autoCreateEnsembleWellLogs() const;

    int     timeStep() const;
    QString wellPathFilePath() const;

    WellPathSource wellPathSource() const;
    void           setWellPathSource( WellPathSource wellPathSource );

    void         setWellPathFromProject( RimWellPath* wellPath );
    RimWellPath* wellPathFromProject() const;

    std::vector<std::pair<QString, RiaDefines::ResultCatType>> properties() const;

    void setCaseData( RigEclipseCaseData* caseData );

protected:
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions,
                                                         bool*                      useOptionsOnly ) override;

    std::vector<RiaDefines::ResultCatType> validResultCategories() const;

private:
    caf::PdmField<caf::FilePath>                m_well;
    caf::PdmField<caf::AppEnum<WellPathSource>> m_wellPathSource;
    caf::PdmPtrField<RimWellPath*>              m_wellPath;
    caf::PdmField<bool>                         m_autoCreateEnsembleWellLogs;

    caf::PdmField<std::vector<QString>> m_selectedKeywords;
    caf::PdmField<int>                  m_timeStep;

    QStringList         m_tabNames;
    RigEclipseCaseData* m_caseData;
};
