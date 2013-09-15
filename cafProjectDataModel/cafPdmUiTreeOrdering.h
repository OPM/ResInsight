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
#include <vector>
#include <QString>

#include "cafPdmUiItem.h"
#include "../cafUserInterface/cafUiTreeItem.h"
#include "cafPdmPointer.h"

namespace caf 
{

class PdmObject;
class PdmFieldHandle;

//typedef UiTreeItem<PdmPointer<PdmObject> > PdmUiTreeItem;

//==================================================================================================
/// Class storing a tree structure representation of some PdmObject hierarchy to be used for tree views in the Gui
//==================================================================================================

class PdmUiTreeOrdering : public UiTreeItem< PdmPointer<PdmObject> >
{
    PdmUiTreeOrdering(PdmUiTreeOrdering* parent = NULL, int position = -1, PdmObject* dataObject = NULL);

    void                add(PdmFieldHandle * field);
    void                add(PdmObject* object);
    PdmUiTreeOrdering*  add(const QString & title, const QString& iconResourceName );

    /// If the rest of the fields containing children is supposed to be omitted, setForgetRemainingFileds to true.
    void                setForgetRemainingFields(bool val)          { m_forgetRemainingFields = val; }
    /// To stop the tree generation at this level, setSubTreeDefined to true
    void                setSubTreeDefined(bool isSubTreeDefined )   { m_isSubTreeDefined = isSubTreeDefined; }

private:
    friend class PdmObject;
    bool                forgetRemainingFields() const       { return m_forgetRemainingFields; }
    bool                isSubTreeDefined() const            { return m_isSubTreeDefined; }
    bool                containsField(PdmFieldHandle* field);

private:
    PdmFieldHandle*     m_field;
    PdmUiItemInfo*      m_uiInfo;

    bool                m_forgetRemainingFields;
    bool                m_isSubTreeDefined;
};



} // End of namespace caf

