
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool readBlockData(QTcpSocket& socket, char* data, quint64 bytesToRead, QStringList& errorMessages)
{
    quint64 bytesRead = 0;
    int blockCount = 0;

    quint64 maxBlockSize = 100000;

    while (bytesRead < bytesToRead)
    {
        if (socket.bytesAvailable())
        {
            quint64 byteCountToRead = qMin(bytesToRead - bytesRead, maxBlockSize);

            qint64 actuallyBytesRead = socket.read(data + bytesRead, byteCountToRead);
            if (actuallyBytesRead < 0)
            {
                errorMessages.push_back("Error detected when writing data, error string from socket");
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

    return true;
}
