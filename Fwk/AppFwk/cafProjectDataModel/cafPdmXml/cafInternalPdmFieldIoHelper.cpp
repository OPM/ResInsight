#include "cafInternalPdmFieldIoHelper.h"

#include <QXmlStreamReader>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldIOHelper::skipCharactersAndComments( QXmlStreamReader& xmlStream )
{
    while ( !xmlStream.atEnd() && ( xmlStream.isCharacters() || xmlStream.isComment() ) )
    {
        xmlStream.readNext();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldIOHelper::skipComments( QXmlStreamReader& xmlStream )
{
    while ( !xmlStream.atEnd() && xmlStream.isComment() )
    {
        xmlStream.readNext();
    }
}

} // End of namespace caf
