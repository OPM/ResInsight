#pragma once

class QXmlStreamReader;

namespace caf
{
class PdmFieldIOHelper
{
public:
    // Utility functions for reading from QXmlStreamReader
    static void skipCharactersAndComments( QXmlStreamReader& xmlStream );
    static void skipComments( QXmlStreamReader& xmlStream );
};

} // End of namespace caf
