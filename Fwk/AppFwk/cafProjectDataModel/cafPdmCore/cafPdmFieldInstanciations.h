/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2014 Ceetron Solutions AS
//
//  <APPLICATION_NAME> is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  <APPLICATION_NAME> is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "cafPdmDataValueField.h"

#include "cafInternalPdmValueFieldSpecializations.h"

extern template class caf::PdmDataValueField<int>;
extern template class caf::PdmDataValueField<bool>;
extern template class caf::PdmDataValueField<QString>;
extern template class caf::PdmDataValueField<double>;

extern template class caf::PdmValueFieldSpecialization<int>;
extern template class caf::PdmValueFieldSpecialization<bool>;
extern template class caf::PdmValueFieldSpecialization<QString>;
extern template class caf::PdmValueFieldSpecialization<double>;
