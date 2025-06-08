/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RimNamedObject.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmField.h"
#include "cafPdmPtrField.h"

#include <set>

class RimCaseCollection;
class RimEclipseCase;
class RimEclipseView;
class RimEclipseViewCollection;
class RimWellTargetMapping;
class RimStatisticsContourMap;

//==================================================================================================
//
//
//
//==================================================================================================
class RimEclipseCaseEnsemble : public RimNamedObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimEclipseCaseEnsemble();
    ~RimEclipseCaseEnsemble() override;

    void addCase( RimEclipseCase* reservoir );
    void removeCase( RimEclipseCase* reservoir );
    bool contains( RimEclipseCase* reservoir ) const;

    RimEclipseCase* findByDescription( const QString& description ) const;
    RimEclipseCase* findByFileName( const QString& gridFileName ) const;

    std::vector<RimEclipseCase*> cases() const;
    std::set<RimEclipseCase*>    casesInViews() const;

    void            addView( RimEclipseView* view );
    RimEclipseView* addViewForCase( RimEclipseCase* eclipseCase );

    std::vector<RimEclipseView*> allViews() const;

    RimEclipseViewCollection* viewCollection() const;

    void                               addWellTargetMapping( RimWellTargetMapping* wellTargetMapping );
    std::vector<RimWellTargetMapping*> wellTargetMappings() const;

    void addStatisticsContourMap( RimStatisticsContourMap* statisticsContourMap );

protected:
    void appendMenuItems( caf::CmdFeatureMenuBuilder& menuBuilder ) const override;
    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    caf::PdmField<int>                                m_groupId;
    caf::PdmChildField<RimCaseCollection*>            m_caseCollection;
    caf::PdmChildField<RimEclipseViewCollection*>     m_viewCollection;
    caf::PdmChildArrayField<RimWellTargetMapping*>    m_wellTargetMappings;
    caf::PdmChildArrayField<RimStatisticsContourMap*> m_statisticsContourMaps;
    caf::PdmPtrField<RimEclipseCase*>                 m_selectedCase;
};
