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

#include "cafPdmUiCoreVec3d.h"

#include "cafPdmField.h"
#include "cafPdmProxyValueField.h"
#include "cafPdmUiLineEditor.h"
#include "cafPdmUiListEditor.h"

namespace caf
{
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiLineEditor, cvf::Vec3d );
CAF_PDM_UI_REGISTER_DEFAULT_FIELD_EDITOR( PdmUiListEditor, std::vector<cvf::Vec3d> );
} // namespace caf

//--------------------------------------------------------------------------------------------------
// If the macro for registering the editor is put as the single statement
// in a cpp file, a dummy static class must be used to make sure the compile unit
// is included
//--------------------------------------------------------------------------------------------------
PdmVec3dInitializer::PdmVec3dInitializer()
{
}
