/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RimWellPath.h"

#include <QString>

class RimWellPathGroup : public RimWellPath
{
    CAF_PDM_HEADER_INIT;

public:
    RimWellPathGroup();

    void                      addChildWellPath( RimWellPath* wellPath );
    std::vector<RimWellPath*> childWellPaths() const;
    size_t                    childWellpathCount() const;
    bool                      hasChildWellPath( RimWellPath* wellPath );
    void                      removeChildWellPath( RimWellPath* wellPath );
    void                      removeAllChildWellPaths();

    void createWellPathGeometry();
    void updateWellPathName();
    void fixBranchNames();
    void makeMoreLevelsIfNecessary();

protected:
    void defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName );

private:
    std::vector<const RigWellPath*> wellPathGeometries() const;
    QString                         createWellPathName() const;

private:
    caf::PdmChildArrayField<RimWellPath*> m_childWellPaths;
};