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

class QString;

namespace caf {


//==================================================================================================
//
// 
//
//==================================================================================================
class Log
{
public:
    //static void info(const char* formatStr, ...);
    static void info(const QString& msg);
    static void warning(const QString& msg);
    static bool error(const QString& err);

    static void infoMultiLine(const QString& line1, const QString& line2Etc);
    static void warningMultiLine(const QString& line1, const QString& line2Etc);
    static bool errorMultiLine(const QString& line1, const QString& line2Etc);

    static void	pumpMessages();
};



}
