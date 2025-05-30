/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimEclipseResultDefinition.h"
#include "RimLegendConfigChangeType.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmChildField.h"
#include "cafPdmPtrField.h"

class RimEclipseCase;
class RimRegularLegendConfig;
class RimTernaryLegendConfig;

//==================================================================================================
///
///
//==================================================================================================
class RimEclipseCellColors : public RimEclipseResultDefinition
{
    CAF_PDM_HEADER_INIT;

public:
    caf::Signal<RimLegendConfigChangeType> legendConfigChanged;

public:
    RimEclipseCellColors();
    ~RimEclipseCellColors() override;

    void            setReservoirView( RimEclipseView* ownerReservoirView );
    RimEclipseView* reservoirView();

    RimRegularLegendConfig* legendConfig();
    RimTernaryLegendConfig* ternaryLegendConfig();

    void setResultVariable( const QString& resultName ) override;

    void updateIconState();

    void updateLegendCategorySettings() override;
    void uiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" );

    void useDiscreteLogLevels( bool enable );

    void setAdditionalUiTreeObjects( const std::vector<caf::PdmObject*>& objects );

protected:
    // Overridden methods
    void fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue ) override;
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "" ) override;

    friend class RimEclipseFaultColors;
    friend class RimCellEdgeColors;
    void initAfterRead() override;

    void defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering ) override;

private:
    void changeLegendConfig( QString resultVarNameOfNewLegend );
    void onLegendConfigChanged( const caf::SignalEmitter* emitter, RimLegendConfigChangeType changeType );
    static RimRegularLegendConfig* createLegendForResult( int caseId, const QString& resultName, bool useDiscreteLevels, bool isCategoryResult );

    caf::PdmChildArrayField<RimRegularLegendConfig*> m_legendConfigData;
    caf::PdmPtrField<RimRegularLegendConfig*>        m_legendConfigPtrField;
    caf::PdmChildField<RimTernaryLegendConfig*>      m_ternaryLegendConfig;

    caf::PdmPointer<RimEclipseView> m_reservoirView;

    std::vector<caf::PdmObject*> m_additionalUiTreeObjects;

    bool m_useDiscreteLogLevels;

    // Obsolete
    caf::PdmChildField<RimRegularLegendConfig*> obsoleteField_legendConfig;
};
