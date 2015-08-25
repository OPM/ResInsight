#include "cafInternalPdmFieldIoHelper.h"

#include <QXmlStreamReader>

namespace caf
{

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmFieldIOHelper::skipCharactersAndComments(QXmlStreamReader& xmlStream)
{
    QXmlStreamReader::TokenType type;
    while (!xmlStream.atEnd() && xmlStream.isCharacters() || xmlStream.isComment())
    {
        type = xmlStream.readNext();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void PdmFieldIOHelper::skipComments(QXmlStreamReader& xmlStream)
{
    QXmlStreamReader::TokenType type;
    while (!xmlStream.atEnd() &&  xmlStream.isComment()) 
    {
         type = xmlStream.readNext();
    }
}

} // End of namespace caf
