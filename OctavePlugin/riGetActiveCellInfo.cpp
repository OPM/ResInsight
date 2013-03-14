#include <QtNetwork>
#include <octave/oct.h>


void getActiveCellInfo(int32NDArray& activeCellInfo, const QString &hostName, quint16 port, QString caseName)
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

    QString command("GetActiveCellInfo ");
    command += caseName;
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

    activeCellCount = byteCount / sizeof(qint32);

    dim_vector dv (2, 1);
    dv(0) = activeCellCount;
    dv(1) = timestepCount;
    activeCellInfo.resize(dv);

    if (!(byteCount && timestepCount))
    {
        error ("Could not find the requested data in ResInsight");
        return;
    }

    // Wait for available data for each column, then read data for each column
    for (size_t tIdx = 0; tIdx < timestepCount; ++tIdx)
    {
        while (socket.bytesAvailable() < (int)byteCount)
        {
            if (!socket.waitForReadyRead(Timeout))
            {
                error((("Waiting for column number: ") + QString::number(tIdx)+  " of 8 : " + socket.errorString()).toLatin1().data());
                octave_stdout << "Active cells: " << activeCellCount << ", Columns: " << timestepCount << std::endl;
                return ;
            }
           OCTAVE_QUIT;
        }

        qint64 bytesRead = 0;
        qint32* internalMatrixData = (qint32*)activeCellInfo.fortran_vec()->mex_get_data();

#if 1 // Use raw data transfer. Faster.
        bytesRead = socket.read((char*)(internalMatrixData + tIdx * activeCellCount), byteCount);
#else
        for (size_t cIdx = 0; cIdx < activeCellCount; ++cIdx)
        {
            socketStream >> internalMatrixData[tIdx * activeCellCount + cIdx];

            if (socketStream.status() == QDataStream::Ok) bytesRead += sizeof(int);
        }
#endif

        if ((int)byteCount != bytesRead)
        {
            error("Could not read binary double data properly from socket");
            octave_stdout << "Active cells: " << activeCellCount << ", Columns: " << timestepCount << std::endl;
        }

        OCTAVE_QUIT;
    }

    QString tmp = QString("riGetActiveCellInfo : Read active cell info");
    if (caseName.isEmpty())
    {
        tmp += QString(" from active case.");
    }
    else
    {
        tmp += QString(" from %1.").arg(caseName);
    }
    octave_stdout << tmp.toStdString() << " Active cells: " << activeCellCount << ", Columns: " << timestepCount << std::endl;


    return;
}



DEFUN_DLD (riGetActiveCellInfo, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetActiveCellInfo( [CaseName/CaseIndex])\n"
           "\n"
           "Returns a two dimentional matrix: [ActiveCells][8]\n"
           "Containing grid and ijk information about the cells from the Eclipse Case defined.\n"
           "The columns contain the following information:\n"
           "  1: GridIndex: The index of the grid the cell resides in. (Main grid has index 0) \n"
           "  2, 3, 4: I, J, K address of the cell in the grid.\n"
           "  5: ParentGridIndex. The index to the grid that this cell's grid is residing in.\n"
           "  6, 7, 8: PI, PJ, PK address of the parent grid cell that this cell is a part of."
           "If the Eclipse Case is not defined, the active View in ResInsight is used."
           )
{
    int nargin = args.length ();
    if (nargin > 1)
    {
        error("riGetActiveCellInfo: Too many arguments. Only the name or index of the case is valid input.\n");
        print_usage();
    }
    else if (nargout < 1)
    {
        error("riGetActiveCellInfo: Missing output argument.\n");
        print_usage();
    }
    else
    {
        int32NDArray propertyFrames;

        if (nargin > 0)
            getActiveCellInfo(propertyFrames, "127.0.0.1", 40001, args(0).char_matrix_value().row_as_string(0).c_str());
        else
            getActiveCellInfo(propertyFrames, "127.0.0.1", 40001, "");

        return octave_value(propertyFrames);
    }

    return octave_value_list ();
}

