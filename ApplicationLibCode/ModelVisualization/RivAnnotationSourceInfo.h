/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#pragma once

#include "RivAnnotationTools.h"

#include "cvfObject.h"
#include "cvfVector3.h"

//==================================================================================================
///
///
//==================================================================================================
class RivAnnotationSourceInfo : public cvf::Object
{
public:
    // Construct an object used to display a single text that can be positioned at several candidate positions. Can be used to display a
    // label for a long and narrow structure like a surface intersection line.
    RivAnnotationSourceInfo( const std::string& text, const std::vector<cvf::Vec3d>& displayCoords );

    // Construct an object used to display a text at a single position. Can be used to display measured depth as labels along a well path.
    RivAnnotationSourceInfo( const std::vector<std::string>& texts, const std::vector<cvf::Vec3d>& displayCoords );

    RivAnnotationTools::LabelPositionStrategy labelPositionStrategyHint() const;
    void                                      setLabelPositionStrategyHint( RivAnnotationTools::LabelPositionStrategy strategy );

    std::string              text() const;
    std::vector<std::string> texts() const;
    std::vector<cvf::Vec3d>  anchorPointsInDisplayCoords() const;

    bool showColor() const;
    void setShowColor( bool showColor );

    cvf::Color3f color() const;
    void         setColor( const cvf::Color3f& color );

private:
    std::string                               m_text;
    bool                                      m_showColor;
    cvf::Color3f                              m_color;
    RivAnnotationTools::LabelPositionStrategy m_labelPositionHint;
    std::vector<cvf::Vec3d>                   m_anchorPointsInDisplayCoords;
    std::vector<std::string>                  m_texts;
};
