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

#include "cafPdmUiTreeOrdering.h"
#include "cafPdmField.h"

namespace caf
{


    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void PdmUiTreeOrdering::add(PdmFieldHandle * field)
    {
        PdmUiTreeOrdering* to = new PdmUiTreeOrdering(this, -1, field->ownerObject());
        to->m_field = field;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    void PdmUiTreeOrdering::add(PdmObject* object)
    {
        PdmUiTreeOrdering* to = new PdmUiTreeOrdering(this, -1, object);
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    PdmUiTreeOrdering* PdmUiTreeOrdering::add(const QString & title, const QString& iconResourceName)
    {
        PdmUiTreeOrdering* to = new PdmUiTreeOrdering(this, -1, NULL);
        to->m_uiInfo = new PdmUiItemInfo(title, QIcon(iconResourceName));
        return to;
    }

    //--------------------------------------------------------------------------------------------------
    /// 
    //--------------------------------------------------------------------------------------------------
    bool PdmUiTreeOrdering::containsField(PdmFieldHandle* field)
    {
        assert (field);
        for (int cIdx = 0; cIdx < this->childCount(); ++cIdx)
        {
            PdmUiTreeOrdering* child = dynamic_cast<PdmUiTreeOrdering*>(this->child(cIdx));

            if (!(child->m_field == field)) 
            {
                return true;
            }
        }

        return false;
    }

    //--------------------------------------------------------------------------------------------------
    ///  Creates an new PdmUiTreeOrdering item, and adds it to parent. If position is -1, it is added 
    ///  at the end of parents existing child list.
    //--------------------------------------------------------------------------------------------------
    PdmUiTreeOrdering::PdmUiTreeOrdering(PdmUiTreeOrdering* parent /*= NULL*/, int position /*= -1*/, PdmObject* dataObject /*= NULL*/) : UiTreeItem< PdmPointer<PdmObject> >(parent, position, dataObject),
        m_field(NULL),
        m_uiInfo(NULL),
        m_forgetRemainingFields(false),
        m_isSubTreeDefined(false)
    {

    }

} //End of namespace caf

