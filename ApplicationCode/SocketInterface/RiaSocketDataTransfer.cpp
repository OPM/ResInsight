/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// 
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RiaSocketDataTransfer.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaSocketDataTransfer::writeBlockDataToSocket(QTcpSocket* socket, const char* data, quint64 bytesToWrite, QStringList& errorMessages)
{
    quint64 bytesWritten = 0;

    quint64 maxBlockSize = maximumValueCountInBlock() * sizeof(double);

    while (bytesWritten < bytesToWrite)
    {
        quint64 byteCountToWrite = qMin(bytesToWrite - bytesWritten, maxBlockSize);

        qint64 actuallyBytesWritten = socket->write(data + bytesWritten, byteCountToWrite);
        if (actuallyBytesWritten < 0)
        {
            errorMessages.push_back("Error detected when writing data, error string from socket");
            errorMessages.push_back(socket->errorString());

            return false;
        }

        bytesWritten += actuallyBytesWritten;

    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaSocketDataTransfer::readBlockDataFromSocket(QTcpSocket* socket, char* data, quint64 bytesToRead, QStringList& errorMessages)
{
    quint64 bytesRead = 0;

    quint64 maxBlockSize = maximumValueCountInBlock() * sizeof(double);

    while (bytesRead < bytesToRead)
    {
        if (socket->bytesAvailable())
        {
            quint64 byteCountToRead = bytesToRead - bytesRead;
            byteCountToRead = qMin(byteCountToRead, maxBlockSize);

            qint64 actuallyBytesRead = socket->read(data + bytesRead, byteCountToRead);
            if (actuallyBytesRead < 0)
            {
                errorMessages.push_back("Error detected when reading data, error string from socket");
                errorMessages.push_back(socket->errorString());

                return false;
            }

            bytesRead += actuallyBytesRead;

#ifdef octave_oct_h
            //octave_stdout << "Byte read " << bytesRead << " of a total of "<< bytesToRead << "\n";
#endif
        }
        else
        {
            if (!socket->waitForReadyRead())
            {
                errorMessages.push_back("Waited for data for %1 milli seconds.");
                errorMessages.push_back(socket->errorString());

                return false;
            }
        }

        // Allow Octave process to end a long running Octave function
#ifdef octave_oct_h
        OCTAVE_QUIT;
#endif

    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
size_t RiaSocketDataTransfer::maximumValueCountInBlock()
{
    return 20000;
}


