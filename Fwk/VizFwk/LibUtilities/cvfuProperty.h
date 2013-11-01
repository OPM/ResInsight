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

#include "cvfCollection.h"
#include "cvfString.h"

namespace cvfu {


//==================================================================================================
//
// Property
//
//==================================================================================================
class Property : public cvf::Object
{
public:
    Property(const cvf::String& name);
    
    const cvf::String&  name() const;

private:
    cvf::String m_name;
};


//==================================================================================================
//
// PropertyBool
//
//==================================================================================================
class PropertyBool : public Property
{
public:
    PropertyBool(const cvf::String& name, bool value);

    bool    value() const;
    void    setValue(bool val);

private:
    bool    m_value;
};


//==================================================================================================
//
// PropertyInt
//
//==================================================================================================
class PropertyInt : public Property
{
public:
    PropertyInt(const cvf::String& name, int value);

    void        setRange(int min, int max);
    int         min() const;
    int         max() const;
    void        setGuiStep(int step);
    int         guiStep() const;

    int         value() const;
    bool        setValue(int val);

private:
    int         m_min;      // Min/max legal values
    int         m_max;      // If min > max, the range is unlimited
    int         m_guiStep;  // Stepping when stepping is allowed in GUI. A value of 0 or less means auto.
    int         m_value;
};


//==================================================================================================
//
// PropertyDouble
//
//==================================================================================================
class PropertyDouble : public Property
{
public:
    PropertyDouble(const cvf::String& name, double value);

    void        setRange(double min, double max);
    double      min() const;
    double      max() const;
    void        setGuiStep(double step);
    double      guiStep() const;
    void        setDecimals(cvf::uint numDecimals);
    cvf::uint   decimals() const;

    double      value() const;
    bool        setValue(double val);

private:
    double      m_min;
    double      m_max;
    double      m_guiStep;
    cvf::uint   m_decimals;
    double      m_value;
};


//==================================================================================================
//
// PropertyEnum
//
//==================================================================================================
class PropertyEnum : public Property
{
public:
    PropertyEnum(const cvf::String& name);

    cvf::uint   itemCount() const;
    void        addItem(const cvf::String& ident, const cvf::String& text);
    void        clearAllItems();
    cvf::String itemText(cvf::uint index) const;

    cvf::uint   currentIndex() const;
    bool        setCurrentIndex(cvf::uint index);
    cvf::String currentIdent() const;
    bool        setCurrentIdent(cvf::String ident);

private:
    std::vector<cvf::String> m_itemIds;
    std::vector<cvf::String> m_itemTexts;
    cvf::uint                m_currentIndex;
};


//==================================================================================================
//
// PropertySet 
//
//==================================================================================================
class PropertySet : public cvf::Object
{
public:
    PropertySet(); 

    size_t      count() const;
    void        add(Property* property);
    Property*   property(size_t index);

private:
    cvf::Collection<Property> m_properties;
};


}

