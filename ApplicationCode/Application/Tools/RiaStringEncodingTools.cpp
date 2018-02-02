/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RiaStringEncodingTools.h"


const std::string RiaStringEncodingTools::toNativeEncoded(const QString& qstring)
{
#ifdef WIN32
    return std::string(qstring.toLatin1().data());
#else
    return std::string(qstring.toUtf8().data());
#endif
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const QString RiaStringEncodingTools::fromNativeEncoded(const char* native)
{
#ifdef WIN32
    return QString::fromLatin1(native);
#else
    return QString::fromUtf8(native);
#endif
}
