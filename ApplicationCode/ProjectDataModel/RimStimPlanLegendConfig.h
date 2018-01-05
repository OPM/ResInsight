/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 -     Statoil ASA
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
#include "cafPdmChildField.h"


class RimLegendConfig;

//==================================================================================================
///  
///  
//==================================================================================================
class RimStimPlanLegendConfig : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RimStimPlanLegendConfig();
    virtual ~RimStimPlanLegendConfig();

    QString name() const;
    void setName(const QString& name);


    virtual caf::PdmFieldHandle* userDescriptionField() override;

protected:
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;
    virtual void defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "") override;

private:
    caf::PdmField<QString>               m_name;
    caf::PdmChildField<RimLegendConfig*> m_legend;

};
