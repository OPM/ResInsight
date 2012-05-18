//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2012 Ceetron AS
//    
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
//##################################################################################################

#pragma once

#include <QTextStream>

#include "cvfLibCore.h"

void operator >> (QTextStream& str, cvf::Color3f& value)
{
    QString text;

    double r, g, b;
    str >> r;
    str >> g;
    str >> b;

    value.set(r, g, b);
}

void operator << (QTextStream& str, const cvf::Color3f& value)
{
    str << value.r() << " " << value.g() << " " << value.b();
}


