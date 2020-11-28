#include "cafInternalPdmFieldIoHelper.h"

#include <QXmlStreamReader>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldIOHelper::skipCharactersAndComments( QXmlStreamReader& xmlStream )
{
    QXmlStreamReader::TokenType type;
    while ( !xmlStream.atEnd() && (xmlStream.isCharacters() || xmlStream.isComment()) )
    {
        type = xmlStream.readNext();
        Q_UNUSED( type );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void PdmFieldIOHelper::skipComments( QXmlStreamReader& xmlStream )
{
    QXmlStreamReader::TokenType type;
    while ( !xmlStream.atEnd() && xmlStream.isComment() )
    {
        type = xmlStream.readNext();
        Q_UNUSED( type );
    }
}

} // End of namespace caf
