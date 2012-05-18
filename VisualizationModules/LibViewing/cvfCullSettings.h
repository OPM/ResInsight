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

namespace cvf {


//==================================================================================================
//
// CullSettings
//
//==================================================================================================
class CullSettings
{
public:
    CullSettings();

    void    enableViewFrustumCulling(bool enable);
    bool    isViewFrustumCullingEnabled() const;

    void    enablePixelSizeCulling(bool enable);
    bool    isPixelSizeCullingEnabled() const;

    void    setPixelSizeCullingAreaThreshold(double pixelArea);
    double  pixelSizeCullingAreaThreshold() const;

private:
    bool    m_viewFrustumCulling;              // Enable/disable view frustum culling
    bool    m_pixelSizeCulling;                // Enable/disable pixel size culling
    double  m_pixelSizeCullingAreaThreshold;   // Objects where the bounding sphere projects to an area smaller than this value will be culled.
};

}
