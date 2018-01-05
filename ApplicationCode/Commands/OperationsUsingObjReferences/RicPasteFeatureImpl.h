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


#include <vector>

class QString;
class QAction;

class RimEclipseCase;
class RimGeoMechCase;
class RimIdenticalGridCaseGroup;

namespace caf 
{
    class PdmObjectGroup;
    class PdmObjectHandle;
} 

//==================================================================================================
/// 
//==================================================================================================
class RicPasteFeatureImpl
{
public:
    static void findObjectsFromClipboardRefs(caf::PdmObjectGroup* objectGroup);

    static RimIdenticalGridCaseGroup* findGridCaseGroup(caf::PdmObjectHandle* objectHandle);
    static RimEclipseCase* findEclipseCase(caf::PdmObjectHandle* objectHandle);
    static RimGeoMechCase* findGeoMechCase(caf::PdmObjectHandle* objectHandle);

    static void setIconAndShortcuts(QAction* action);

    static void clearClipboard();

private:
    static void populateObjectGroupFromReferences(const std::vector<QString>& referenceList, caf::PdmObjectGroup* objectGroup);
    static void referencesFromClipboard(std::vector<QString>& referenceList);
};



