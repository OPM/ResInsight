//##################################################################################################
//
//   Custom Visualization Core library
//   Copyright (C) 2011-2013 Ceetron AS
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

#include "cvfModel.h"

namespace cvf {

class Frustum;


//==================================================================================================
//
// A simple model implementation using a list of parts.
//
//==================================================================================================
class ModelBasicList : public Model
{
public:
    virtual String          name() const;
    void                    setName(const String& name);

    void                    addPart(Part* part);
    Part*                   part(size_t index);
    size_t                  partCount() const;
    void                    removePart(Part* part);
    void                    removeAllParts();
    void                    shrinkPartCount(size_t newPartCount);

    virtual void            findVisibleParts(PartRenderHintCollection* visibleParts, const Camera& camera, const CullSettings& cullSettings, uint enableMask);
    virtual void            allParts(Collection<Part>* partCollection);
    virtual void            mergeParts(double maxExtent, uint minimumPrimitiveCount);

    virtual void            updateBoundingBoxesRecursive();
    virtual BoundingBox     boundingBox() const;

    virtual bool            rayIntersect(const RayIntersectSpec& rayIntersectSpec, HitItemCollection* hitItemCollection);

    Part*                   findPartByID(int64 id);
    Part*                   findPartByName(String name);

private:
    ref<Part>               mergeAndAddPart(Collection<Part>& partCollection) const;

    bool                    partVisible(cvf::Part* part, const Camera* camera, const CullSettings* cullSettings, uint enableMask, double* projectedAreaPixels);

protected:
    String           m_modelName;
    Collection<Part> m_parts;
    BoundingBox      m_boundingBox;
};

}
