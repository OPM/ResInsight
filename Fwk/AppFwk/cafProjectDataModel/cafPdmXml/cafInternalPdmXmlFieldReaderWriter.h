#pragma once

#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "cafInternalPdmFieldIoHelper.h"
#include "cafInternalPdmFilePathStreamOperators.h"
#include "cafInternalPdmStreamOperators.h"

namespace caf
{
class PdmObjectFactory;
template <typename T>
class PdmPointer;

//--------------------------------------------------------------------------------------------------
/// Generic write method for fields. Will work as long as DataType supports the stream operator
/// towards a QTextStream. Some special datatype should not specialize this method unless it is
/// impossible/awkward to implement the stream operator
/// Implemented in a proxy class to allow  partial specialization
//--------------------------------------------------------------------------------------------------
template <typename DataType>
struct PdmFieldWriter
{
    static void writeFieldData( const DataType& fieldValue, QXmlStreamWriter& xmlStream )
    {
        QString     dataString;
        QTextStream data( &dataString, QIODevice::WriteOnly );

        // Use precision of 15 to cover most value ranges for double values
        // Default Qt behavior is precision of 6
        data.setRealNumberPrecision( 15 );

        data << fieldValue;
        xmlStream.writeCharacters( dataString );
    }
};

template <typename DataType>
struct PdmFieldReader
{
    static void readFieldData( DataType& fieldValue, QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory );
};

//--------------------------------------------------------------------------------------------------
/// Generic read method for fields. Will work as long as DataType supports the stream operator
/// towards a QTextStream. Some special datatype should not specialize this method unless it is
/// impossible/awkward to implement the stream operator
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void PdmFieldReader<DataType>::readFieldData( DataType& fieldValue, QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory )
{
    PdmFieldIOHelper::skipComments( xmlStream );
    if ( !xmlStream.isCharacters() ) return;

    QString     dataString = xmlStream.text().toString();
    QTextStream data( &dataString, QIODevice::ReadOnly );
    data >> fieldValue;

    // Make stream point to end of element
    QXmlStreamReader::TokenType type = xmlStream.readNext();
    Q_UNUSED( type );
    PdmFieldIOHelper::skipCharactersAndComments( xmlStream );
}

//--------------------------------------------------------------------------------------------------
/// Specialized read function for QStrings, because the >> operator only can read word by word
//--------------------------------------------------------------------------------------------------
template <>
void PdmFieldReader<QString>::readFieldData( QString& field, QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory );

} // End of namespace caf
