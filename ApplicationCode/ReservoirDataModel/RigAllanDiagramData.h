/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
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

#include "cvfObject.h"
#include <QString>

#include <map>

class RigAllanDiagramData : public cvf::Object
{
public:
    RigAllanDiagramData();
    ~RigAllanDiagramData() override;

    const std::map<std::pair<int, int>, int>& formationCombinationToCategory()
    {
        return m_formationCombinationToCategory;
    }

    std::pair<int, int> formationIndexCombinationFromCategory( int category )
    {
        for ( auto it : m_formationCombinationToCategory )
        {
            if ( it.second == category )
            {
                return it.first;
            }
        }

        return std::make_pair( -1, -1 );
    }

    void setFormationCombinationToCategorymap( const std::map<std::pair<int, int>, int>& mapping )
    {
        m_formationCombinationToCategory = mapping;
    }

private:
    std::map<std::pair<int, int>, int> m_formationCombinationToCategory;
};
