#include "cafPdmFieldHandle.h"

#include "cafPdmFieldCapability.h"


namespace caf
{

#if 0
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmFieldHandle::assertValid() const
{
    if (m_keyword == "UNDEFINED")
    {
        std::cout << "PdmField: Detected use of non-initialized field. Did you forget to do CAF_PDM_InitField() on this field ?\n";
        return false;
    }

    if (!PdmXmlSerializable::isValidXmlElementName(m_keyword))
    {
        std::cout << "PdmField: The supplied keyword: \"" << m_keyword.toStdString() << "\" is an invalid XML element name, and will break your file format!\n";
        return false;
    }

    return true;
}
#endif

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmFieldHandle::setKeyword(const QString& keyword)
{
    m_keyword = keyword;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmFieldHandle::hasChildObjects()
{
    std::vector<PdmObjectHandle*> children;
    this->childObjects(&children);
    return (children.size() > 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
PdmFieldHandle::~PdmFieldHandle()
{
    for (size_t i = 0; i < m_capabilities.size(); ++i)
    {
        if (m_capabilities[i].second) delete m_capabilities[i].first;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool PdmFieldHandle::hasPtrReferencedObjects()
{
    std::vector<PdmObjectHandle*> ptrReffedObjs;
    this->ptrReferencedObjects(&ptrReffedObjs);
    return (ptrReffedObjs.size() > 0);
}

// These two functions can be used when PdmCore is used standalone without PdmUi/PdmXml
/*
PdmUiFieldHandle* PdmFieldHandle::uiCapability()
{
    return NULL;
}

PdmXmlFieldHandle* PdmFieldHandle::xmlCapability()
{
    return NULL;
}
*/

} // End of namespace caf
