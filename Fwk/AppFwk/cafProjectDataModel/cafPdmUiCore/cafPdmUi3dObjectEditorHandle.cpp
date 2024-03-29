//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) Ceetron Solutions AS
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

#include "cafPdmUi3dObjectEditorHandle.h"

//==================================================================================================
///
///
///
//==================================================================================================
namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUi3dObjectEditorHandle::PdmUi3dObjectEditorHandle()
    : m_isInComparisonView( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
PdmUi3dObjectEditorHandle::~PdmUi3dObjectEditorHandle()
{
}

//--------------------------------------------------------------------------------------------------
/// The viewer widget set here is provided by the PdmUiSelection3dEditorVisualizer and is expected
/// to be cast able to whatever is needed in subclasses.
/// Not allowed to change. Should be constructor argument, but makes factory stuff difficult.
//--------------------------------------------------------------------------------------------------
void PdmUi3dObjectEditorHandle::setViewer( QWidget* ownerViewer, bool inComparisonView )
{
    CAF_ASSERT( m_ownerViewer.isNull() );
    m_ownerViewer        = ownerViewer;
    m_isInComparisonView = inComparisonView;
}

} // namespace caf
