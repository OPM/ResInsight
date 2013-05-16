#include <QtNetwork>
#include <octave/oct.h>


void getEclipseProperty(Matrix& propertyFrames, const QString &hostName, quint16 port, QString caseName, QString propertyName)
{
    QString serverName = hostName;
    quint16 serverPort = port;

    const int Timeout = 5 * 1000;

    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(Timeout))
    {
        error((("Connection: ") + socket.errorString()).toLatin1().data());
        return;
    }

    // Create command and send it:

    QString command("GetProperty ");
    command += caseName + " " + propertyName;
    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(QDataStream::Qt_4_0);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)(2*sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(Timeout))
        {
            error((("Wating for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

    // Read timestep count and blocksize

    quint64 timestepCount;
    quint64 byteCount;
    size_t  activeCellCount;

    socketStream >> timestepCount;
    socketStream >> byteCount;

    activeCellCount = byteCount / sizeof(double);
    propertyFrames.resize(activeCellCount, timestepCount);

    if (!(byteCount && timestepCount))
    {
        error ("Could not find the requested data in ResInsight");
        return;
    }

    // Wait for available data for each timestep, then read data for each timestep
    for (size_t tIdx = 0; tIdx < timestepCount; ++tIdx)
    {
        while (socket.bytesAvailable() < (int)byteCount)
        {
            if (!socket.waitForReadyRead(Timeout))
            {
                error((("Waiting for timestep data number: ") + QString::number(tIdx)+  ": " + socket.errorString()).toLatin1().data());
                octave_stdout << "Active cells: " << activeCellCount << ", Timesteps: " << timestepCount << std::endl;
                return ;
            }
           OCTAVE_QUIT;
        }

        qint64 bytesRead = 0;
        double * internalMatrixData = propertyFrames.fortran_vec();

#if 1 // Use raw data transfer. Faster.
        bytesRead = socket.read((char*)(internalMatrixData + tIdx * activeCellCount), byteCount);
#else
        for (size_t cIdx = 0; cIdx < activeCellCount; ++cIdx)
        {
            socketStream >> internalMatrixData[tIdx * activeCellCount + cIdx];

            if (socketStream.status() == QDataStream::Ok) bytesRead += sizeof(double);
        }
#endif

        if ((int)byteCount != bytesRead)
        {
            error("Could not read binary double data properly from socket");
            octave_stdout << "Active cells: " << activeCellCount << ", Timesteps: " << timestepCount << std::endl;
        }

        OCTAVE_QUIT;
    }

    QString tmp = QString("riGetActiveCellProperty : Read %1").arg(propertyName);

    if (caseName == "-1")
    {
        tmp += QString(" from active case.");
    }
    else
    {
        tmp += QString(" from %1.").arg(caseName);
    }
    octave_stdout << tmp.toStdString() << " Active cells : " << activeCellCount << ", Timesteps : " << timestepCount << std::endl;

    return;
}



DEFUN_DLD (riGetActiveCellProperty, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetActiveCellProperty( [CaseName/CaseIndex], PropertyName )\n"
           "\n"
           "Returns a two dimentional matrix: [ActiveCells][Timesteps]\n"
           "Containing the requested property data from the Eclipse Case defined.\n"
           "If the Eclipse Case is not defined, the active View in ResInsight is used."
           )
{
    int nargin = args.length ();
    if (nargin < 1)
    {
        error("riGetActiveCellProperty: Too few arguments. The name of the property requested is neccesary.\n");
        print_usage();
    }
    else if (nargout < 1)
    {
        error("riGetActiveCellProperty: Missing output argument.\n");
        print_usage();
    }
    else
    {
        Matrix propertyFrames;

        if (nargin > 1)
            getEclipseProperty(propertyFrames, "127.0.0.1", 40001, args(0).char_matrix_value().row_as_string(0).c_str(), args(1).char_matrix_value().row_as_string(0).c_str());
        else
            getEclipseProperty(propertyFrames, "127.0.0.1", 40001, "-1", args(0).char_matrix_value().row_as_string(0).c_str());

        return octave_value(propertyFrames);
    }

    return octave_value_list ();
}
