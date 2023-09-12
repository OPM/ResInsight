/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023 Equinor ASA
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

#include "cvfCollection.h"
#include "cvfColor3.h"
#include "cvfObject.h"
#include "cvfVector3.h"

#include <string>
#include <vector>

namespace cvf
{
class Part;
class Camera;
class ModelBasicList;
class DrawableText;
class Font;
} // namespace cvf

class RivAnnotationTools
{
public:
    enum class LabelPositionStrategy
    {
        RIGHT,
        LEFT,
        LEFT_AND_RIGHT,
        COUNT_HINT,
        ALL,
        NONE,
        UNDEFINED
    };

    RivAnnotationTools();

    // By default, each annotation object has a label position strategy. Use this method to override the default.
    void setOverrideLabelPositionStrategy( LabelPositionStrategy strategy );

    // When using COUNT_HINT, this number is used to determine the number of labels to show.
    void setCountHint( int countHint );

    // Create labels for the given collection of parts. The labels are added to the given model.
    void addAnnotationLabels( const cvf::Collection<cvf::Part>& partCollection, const cvf::Camera* camera, cvf::ModelBasicList* model );

    static cvf::ref<cvf::Part> createPartFromPolyline( const cvf::Color3f& color, const std::vector<cvf::Vec3d>& polyLine );

    static cvf::ref<cvf::DrawableText> createDrawableText( cvf::Font*         font,
                                                           cvf::Color3f       textColor,
                                                           cvf::Color3f       backgroundColor,
                                                           const std::string& text,
                                                           const cvf::Vec3f&  position );

    static cvf::ref<cvf::Part> createPart( cvf::DrawableText* drawableText );

private:
    LabelPositionStrategy m_overrideStrategy;
    int                   m_labelCountHint;
};
