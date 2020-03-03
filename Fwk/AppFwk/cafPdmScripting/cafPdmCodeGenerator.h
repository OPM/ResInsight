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
#pragma once

#include "cafFactory.h"

#include <QString>

#define CAF_PDM_CODE_GENERATOR_HEADER_INIT \
    public: \
    static const std::string& fileExtension();

#define CAF_PDM_CODE_GENERATOR_SOURCE_INIT(ClassName, FileExtension)\
    const std::string& ClassName::fileExtension() { static std::string ext = FileExtension; return ext;} \
    CAF_FACTORY_REGISTER(caf::PdmCodeGenerator, ClassName, std::string, ClassName::fileExtension())


namespace caf {

class PdmObjectFactory;

//==================================================================================================
/// Abstract skeleton code generator for the Project Data Model
//==================================================================================================
class PdmCodeGenerator
{
public:
    virtual QString generate(PdmObjectFactory* factory) const = 0;
};


typedef Factory<PdmCodeGenerator, std::string> PdmCodeGeneratorFactory;

}