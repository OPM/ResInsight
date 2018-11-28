/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RigMainGrid.h"

#include <QString>

class RigActiveCellInfo;
class RigFemPartCollection;
class RigMainGrid;
class RimCase;
class Rim3dView;

namespace cvf
{
class StructGridInterface;
}

class QString;

//==================================================================================================
///
///
//==================================================================================================
class RigReservoirGridTools
{
public:
    static int                             gridCount(RimCase* rimCase);
    static const cvf::StructGridInterface* mainGrid(RimCase* rimCase);
    static const cvf::StructGridInterface* gridByIndex(RimCase* rimCase, int gridIndex);
    static QString                         gridName(RimCase* rimCase, int gridIndex);

    static const RigActiveCellInfo* activeCellInfo(Rim3dView* rimView);

    template<class InputIterator>
    static QString globalCellIndicesToOneBasedIJKText(InputIterator first, InputIterator last, const RigMainGrid* mainGrid)
    {
        QString txt;

        if (mainGrid)
        {
            while (first != last)
            {
                size_t globalCellIndex = *first;
                size_t i, j, k;
                mainGrid->ijkFromCellIndex(globalCellIndex, &i, &j, &k);
                i++;
                j++;
                k++;

                QString itemText = QString("%1 %2 %3").arg(i).arg(j).arg(k);

                txt += itemText + "\n";

                ++first;
            }
        }

        return txt;
    }

private:
    static RigMainGrid*          eclipseMainGrid(RimCase* rimCase);
    static RigFemPartCollection* geoMechPartCollection(RimCase* rimCase);
};
