#include "cafPdmXmlFieldHandle.h"

#include "cafAssert.h"
#include "cafPdmFieldHandle.h"
#include "cafPdmXmlObjectHandle.h"

#include <iostream>

namespace caf
{


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmXmlFieldHandle::assertValid() const
{

    if (m_owner->keyword().isEmpty())
    {
        std::cout << "PdmField: Detected use of non-initialized field. Did you forget to do CAF_PDM_InitField() on this field ?\n";
        return false;
    }

    if (!PdmXmlObjectHandle::isValidXmlElementName(m_owner->keyword()))
    {
        std::cout << "PdmField: The supplied keyword: \"" << m_owner->keyword().toStdString() << "\" is an invalid XML element name, and will break your file format!\n";
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmXmlFieldHandle::PdmXmlFieldHandle(PdmFieldHandle* owner, bool giveOwnership)
    : m_isIOReadable(true), m_isIOWritable(true), m_isCopyable(true)
{
    m_owner = owner; 
    owner->addCapability(this, giveOwnership);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmXmlFieldHandle::disableIO()
{
    setIOReadable(false);
    setIOWritable(false);
    setCopyable(false);
}

//--------------------------------------------------------------------------------------------------
/// Returns the classKeyword of the child class type, if this field is supposed to contain pointers 
/// to PdmObjectHandle derived onbjects. 
/// Returns empty string if the field is not containig some PdmObjectHandle type
//--------------------------------------------------------------------------------------------------
QString PdmXmlFieldHandle::childClassKeyword()
{
    return m_childClassKeyword;
}

//--------------------------------------------------------------------------------------------------
/// Implementation of uiCapability() defined in cafPdmFieldHandle.h
//--------------------------------------------------------------------------------------------------
PdmXmlFieldHandle* PdmFieldHandle::xmlCapability()
{
    PdmXmlFieldHandle* xmlField = capability<PdmXmlFieldHandle>();
    CAF_ASSERT(xmlField);

    return xmlField;
}


} // End of namespace caf
