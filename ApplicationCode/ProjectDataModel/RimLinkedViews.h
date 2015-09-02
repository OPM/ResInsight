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

#include "RimDefines.h"

#include "cafPdmChildArrayField.h"
#include "cafPdmField.h"
#include "cafPdmObject.h"
#include "cafPdmPtrField.h"

class RimManagedViewConfig;
class RiuViewer;
class RimView;

//==================================================================================================
///  
///  
//==================================================================================================
class RimLinkedViews : public caf::PdmObject
{
     CAF_PDM_HEADER_INIT;

public:
    RimLinkedViews(void);
    virtual ~RimLinkedViews(void);

    void     setMainView(RimView* view);
    RimView* mainView();

    caf::PdmChildArrayField<RimManagedViewConfig*> viewConfigs;

    void applyAllOperations();

    void updateTimeStep(RimView* sourceView, int timeStep);
    void updateCellResult();

    void updateRangeFilters();
    void updatePropertyFilters();

    void configureOverrides();

    void allViewsForCameraSync(std::vector<RimView*>& views);
    void allViews(std::vector<RimView*>& views);

public:
    static QString  displayNameForView(RimView* view);
    RimManagedViewConfig* viewConfigForView(RimView* view);

protected:
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly);
    virtual caf::PdmFieldHandle*            userDescriptionField()  { return &m_name; }
    virtual void                            defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName = "");
    virtual void                            initAfterRead();

private:
    caf::PdmPtrField<RimView*>  m_mainView;
    caf::PdmField<QString>      m_name;

};
