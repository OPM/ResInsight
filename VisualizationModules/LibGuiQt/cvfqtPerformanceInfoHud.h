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

#include <QtCore/QStringList>

namespace cvf {
    class PerformanceInfo;
    class OpenGLResourceManager;
    class Camera;
}

class QPainter;

namespace cvfqt {


//==================================================================================================
//
// PerformanceInfoHud 
//
//==================================================================================================
class PerformanceInfoHud 
{
public:
    PerformanceInfoHud();

    void    addStrings(const cvf::PerformanceInfo& performanceInfo);
    void    addStrings(const cvf::OpenGLResourceManager& resourceManager);
    void    addStrings(const cvf::Camera& camera);
    void    addString(const QString& str);

    void    draw(QPainter *painter, int widgetWidth, int widgetHeight);

private:
    QStringList m_drawStrings;  // The list of strings that will be drawn
};

}
