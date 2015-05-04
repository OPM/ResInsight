/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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
#include "cafPdmPointer.h"
#include "cafAppEnum.h"

class RimLegendConfig;
class RimGeoMechView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimGeoMechResultSlot : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimGeoMechResultSlot(void);
    virtual ~RimGeoMechResultSlot(void);

    void                    setReservoirView(RimGeoMechView* ownerReservoirView);
    RifGeoMechReaderInterface* resultReaderInterface();

    caf::PdmField<RimLegendConfig*> legendConfig;


    enum ResultPositionEnum {
        NODAL,
        ELEMENT_NODAL,
        INTEGRATION_POINT
    };

private:
    virtual QList<caf::PdmOptionItemInfo>            calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, 
                                                                           bool * useOptionsOnly);
    std::map<std::string, std::vector<std::string> > getResultMetaDataForUIFieldSetting();

    virtual void fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue);
    static void getUiAndResultVariableStringList(QStringList* uiNames, QStringList* variableNames, 
                                                 const std::map<std::string, std::vector<std::string> >& fieldCompNames);
    static QString     composeUiVarString(const QString& resultFieldName, const QString& resultComponentName);

    virtual void initAfterRead();
    void loadResult();

    caf::PdmField<caf::AppEnum<ResultPositionEnum> > m_resultPositionType;
    caf::PdmField<QString>                           m_resultFieldName;
    caf::PdmField<QString>                           m_resultComponentName;

    caf::PdmField<caf::AppEnum<ResultPositionEnum> > m_resultPositionTypeUiField;
    caf::PdmField<QString>                           m_resultVariableUiField;

    caf::PdmPointer<RimGeoMechView>                  m_reservoirView;
};
