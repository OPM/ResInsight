/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "cafPdmField.h"
#include "cafPdmObject.h"

#include <QString>

class RimWellPath;
class RimKeywordWconprod;
class RimKeywordWconinje;

//==================================================================================================
///
///
//==================================================================================================
class RimJobWellSettings : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimJobWellSettings();
    ~RimJobWellSettings() override;

    enum class WellOpenType
    {
        OPEN_BY_POSITION,
        OPEN_AT_DATE
    };

    void uiOrdering( caf::PdmUiGroup* uiGroup );

    void setWellPath( RimWellPath* wellPath );
    void setWellGroupName( const QString& wellGroupName );
    void setOpenTimeStep( int openTimeStep );
    void setWellOpenType( WellOpenType wellOpenType );
    void setWellOpenKeyword( const QString& wellOpenKeyword );
    void setWconprodKeyword( RimKeywordWconprod* wconprodKeyword );
    void setWconinjeKeyword( RimKeywordWconinje* wconinjeKeyword );
    void setIncludeMSWData( bool includeMSWData );

    bool                addNewWell() const;
    RimWellPath*        wellPath() const;
    QString             wellGroupName() const;
    int                 openTimeStep() const;
    WellOpenType        wellOpenType() const;
    QString             wellOpenKeyword() const;
    RimKeywordWconprod* wconprodKeyword() const;
    RimKeywordWconinje* wconinjeKeyword() const;
    bool                includeMSWData() const;

    void useDateStrings( const std::vector<QString>& dateStrings );
    void useWellGroups( const std::vector<QString>& wellGroups );

protected:
    QList<caf::PdmOptionItemInfo> calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions ) override;

private:
    caf::PdmField<bool>                       m_addNewWell;
    caf::PdmPtrField<RimWellPath*>            m_wellPath;
    caf::PdmField<QString>                    m_wellGroupName;
    caf::PdmField<int>                        m_openTimeStep;
    caf::PdmField<caf::AppEnum<WellOpenType>> m_wellOpenType;
    caf::PdmField<QString>                    m_wellOpenKeyword;
    caf::PdmChildField<RimKeywordWconprod*>   m_wconprodKeyword;
    caf::PdmChildField<RimKeywordWconinje*>   m_wconinjeKeyword;
    caf::PdmField<bool>                       m_includeMSWData;

    std::vector<QString> m_dateStrings;
    std::vector<QString> m_wellGroups;
};
