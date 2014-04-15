
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool readBlockData_to_be_deleted(QTcpSocket& socket, char* data, quint64 bytesToRead, QStringList& errorMessages)
{
    quint64 bytesRead = 0;
    int blockCount = 0;

    quint64 maxBlockSize = riOctavePlugin::socketMaxByteCount;

    while (bytesRead < bytesToRead)
    {
        if (socket.bytesAvailable())
        {
            quint64 byteCountToRead = qMin(bytesToRead - bytesRead, maxBlockSize);

            qint64 actuallyBytesRead = socket.read(data + bytesRead, byteCountToRead);
            if (actuallyBytesRead < 0)
            {
                errorMessages.push_back("Error detected when reading data, error string from socket :");
                errorMessages.push_back(socket.errorString());

                return false;
            }

            bytesRead += actuallyBytesRead;

            octave_stdout << "Bytes read " << bytesRead << " of total " << bytesToRead << std::endl;

            blockCount++;
        }
        else
        {
            if (!socket.waitForReadyRead())
            {
                errorMessages.push_back("Waited for data for %1 milli seconds.");
                errorMessages.push_back(socket.errorString());

                return false;
            }
        }
    }
    
    octave_stdout << "Bytes read " << bytesToRead << std::endl;

    return true;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool writeBlockData_to_be_deleted(QTcpSocket& socket, const char* data, quint64 bytesToWrite, QStringList& errorMessages)
{
    quint64 bytesWritten = 0;
    int blockCount = 0;

    quint64 maxBlockSize = riOctavePlugin::socketMaxByteCount;

    while (bytesWritten < bytesToWrite)
    {
        quint64 byteCountToWrite = qMin(bytesToWrite - bytesWritten, maxBlockSize);

        qint64 actuallyBytesWritten = socket.write(data + bytesWritten, byteCountToWrite);
        if (actuallyBytesWritten < 0)
        {
            errorMessages.push_back("Error detected when writing data, error string from socket");
            errorMessages.push_back(socket.errorString());

            return false;
        }

        bytesWritten += actuallyBytesWritten;

        blockCount++;
    }

    return true;
}
