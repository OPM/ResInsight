/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RimCellFilter.h"


//==================================================================================================
///  
///  
//==================================================================================================
class RimPropertyFilter : public RimCellFilter
{
    CAF_PDM_HEADER_INIT;
public:
    RimPropertyFilter();
    virtual ~RimPropertyFilter();

    std::vector<int>                        selectedCategoryValues() const;

protected:
    void                                    setCategoryValues(const std::vector<int>& categoryValues);
    void                                    setCategoryNames(const std::vector<QString>& categoryNames);
    void                                    setCategoryNamesAndValues(const std::vector<std::pair<QString, int>>& categoryNamesAndValues);
    void                                    clearCategories();
    
    virtual QList<caf::PdmOptionItemInfo>   calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly) override;

protected:
    caf::PdmField< std::vector<int> >   m_selectedCategoryValues;
    
    std::vector<int>                    m_categoryValues;
    std::vector<QString>                m_categoryNames;
};
