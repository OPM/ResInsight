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

class RiuSelectionItem;
class RiuEclipseSelectionItem;
class RiuGeoMechSelectionItem;

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class RiuSelectionChangedHandler
{
public:
    RiuSelectionChangedHandler();
    ~RiuSelectionChangedHandler();

    void handleSelectionDeleted() const;
    void handleItemAppended(const RiuSelectionItem* item) const;
    void handleSetSelectedItem(const RiuSelectionItem* item) const;

private:
    void addCurveFromSelectionItem(const RiuSelectionItem* itemAdded) const;
    void addCurveFromSelectionItem(const RiuEclipseSelectionItem* selectionItem) const;
    void addCurveFromSelectionItem(const RiuGeoMechSelectionItem* selectionItem) const;
    void updateRelativePermeabilityPlot(const RiuSelectionItem* selectionItem) const;
    void updatePvtPlot(const RiuSelectionItem* selectionItem) const;

    void scheduleUpdateForAllVisibleViews() const;
    void updateResultInfo(const RiuSelectionItem* itemAdded) const;
};

