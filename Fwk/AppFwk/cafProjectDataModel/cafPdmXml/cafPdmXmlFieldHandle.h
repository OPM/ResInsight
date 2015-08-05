#pragma once

#include "cafPdmFieldCapability.h"
#include <QString>

class QXmlStreamReader;
class QXmlStreamWriter;

namespace caf
{

class PdmFieldHandle;
class PdmObjectFactory;
class PdmReferenceHelper;


//==================================================================================================
//
// 
//
//==================================================================================================
class PdmXmlFieldHandle : public PdmFieldCapability
{
public:
    PdmXmlFieldHandle(PdmFieldHandle* owner , bool giveOwnership);
    virtual ~PdmXmlFieldHandle() { }

    PdmFieldHandle* fieldHandle() { return m_owner; }
private:
    PdmFieldHandle* m_owner;

    /// Xml Serialization
public:
    virtual void     readFieldData(QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory)  = 0;
    virtual void     writeFieldData(QXmlStreamWriter& xmlStream) = 0;

    bool             isIOReadable()                                 { return m_isIOReadable; }
    bool             isIOWritable()                                 { return m_isIOWritable; }
    void             setIOWritable(bool isWritable)                 { m_isIOWritable = isWritable; }
    void             setIOReadable(bool isReadable)                 { m_isIOReadable = isReadable; }

    QString          childClassKeyword();
protected:
    bool             assertValid() const;
    QString          m_childClassKeyword; ///< Must be set in constructor of derived XmlFieldHandle

private:
    bool             m_isIOReadable;
    bool             m_isIOWritable;

 
};

} // End of namespace caf

#include "cafInternalPdmXmlFieldCapability.h"
