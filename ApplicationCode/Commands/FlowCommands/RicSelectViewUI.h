/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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
#include "cafPdmPtrField.h"

class RimEclipseView;
class RimEclipseResultCase;

//==================================================================================================
/// 
//==================================================================================================
class RicSelectViewUI : public caf::PdmObject
{
    CAF_PDM_HEADER_INIT;

public:
    RicSelectViewUI();

    void            setView(RimEclipseView* currentView);
    void            setCase(RimEclipseResultCase* currentCase);

    RimEclipseView* selectedView() const;
    bool            createNewView() const;
    QString         newViewName() const;
    
    virtual QList<caf::PdmOptionItemInfo> calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly) override;

protected:
    virtual void defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering) override;

private:
    caf::PdmPtrField<RimEclipseView*>   m_selectedView;
    caf::PdmField<bool>                 m_createNewView;
    caf::PdmField<QString>              m_newViewName;

    RimEclipseView*         m_currentView;
    RimEclipseResultCase*   m_currentCase;
};

