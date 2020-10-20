#include "cafInternalPdmXmlFieldReaderWriter.h"

#include "cafInternalPdmFieldIoHelper.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
/// Specialized read function for QStrings, because the >> operator only can read word by word
//--------------------------------------------------------------------------------------------------
template <>
void PdmFieldReader<QString>::readFieldData( QString& field, QXmlStreamReader& xmlStream, PdmObjectFactory* )
{
    PdmFieldIOHelper::skipComments( xmlStream );
    if ( !xmlStream.isCharacters() ) return;

    field = xmlStream.text().toString();

    // Make stream point to end of element
    QXmlStreamReader::TokenType type;
    type = xmlStream.readNext();
    PdmFieldIOHelper::skipCharactersAndComments( xmlStream );
}

} // End of namespace caf
