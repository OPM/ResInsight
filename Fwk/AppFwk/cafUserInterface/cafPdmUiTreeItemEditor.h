//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
//
//   This library may be used under the terms of either the GNU General Public License or
//   the GNU Lesser General Public License as follows:
//
//   GNU General Public License Usage
//   This library is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU General Public License at <<http://www.gnu.org/licenses/gpl.html>>
//   for more details.
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################

#pragma once
#include "cafPdmUiEditorHandle.h"

namespace caf
{
class PdmUiTreeEditorHandle;

//==================================================================================================
/// A class that tells the treeViewEditor to update the sub tree of its associate PdmUiItem, when
/// needed. (Typically because of a direct call to updateConnectedEditors() )
//==================================================================================================

class PdmUiTreeItemEditor : public PdmUiEditorHandle
{
public:
    explicit PdmUiTreeItemEditor( PdmUiItem* uiItem );
    ~PdmUiTreeItemEditor() override{};

    void setTreeViewEditor( PdmUiTreeEditorHandle* treeViewEditor ) { m_treeViewEditor = treeViewEditor; }

protected: // Interface to override:
    void configureAndUpdateUi( const QString& uiConfigName ) override;

private:
    PdmUiTreeEditorHandle* m_treeViewEditor;
};

} // End of namespace caf
