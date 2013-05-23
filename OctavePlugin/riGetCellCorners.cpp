#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"


void getCellCorners(NDArray& cellCornerValues, const QString &hostName, quint16 port, const qint32& caseId, const quint32& gridIndex)
{
    QString serverName = hostName;
    quint16 serverPort = port;

    const int timeout = riOctavePlugin::timeOutMilliSecs;

    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(timeout))
    {
        error((("Connection: ") + socket.errorString()).toLatin1().data());
        return;
    }

    // Create command and send it:

    QString command = QString("GetCellCorners %1 %2").arg(caseId).arg(gridIndex);
    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)(5 * sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(timeout))
        {
            error((("Wating for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

    // Read timestep count and blocksize

    quint64 cellCountI;
    quint64 cellCountJ;
    quint64 cellCountK;
    quint64 cellCount;
    quint64 byteCount;

    socketStream >> cellCount;
    socketStream >> cellCountI;
    socketStream >> cellCountJ;
    socketStream >> cellCountK;
    socketStream >> byteCount;

    // Create a 4D matrix, with the a column with the tree double value coords running as fastest index, then I, J, K
    // Octave script to access coords
    //   coords = riGetCellCenters
    //   coords(:,i, j, k) # Will return the coords for given ijk location
    dim_vector dv;
    dv.resize(5);
    dv(0) = 3;
    dv(1) = 8;
    dv(2) = cellCountI;
    dv(3) = cellCountJ;
    dv(4) = cellCountK;
    cellCornerValues.resize(dv);

//    octave_stdout << "GetCellCorners - coord count: " << coordCount << ", byteCount: " << byteCount << std::endl;

    if (!(byteCount && cellCount))
    {
        error ("Could not find the requested data in ResInsight");
        return;
    }

    octave_stdout << "GetCellCorners - before wait for data" << std::endl;

    // Wait for available data for each column, then read data for each column
    while (socket.bytesAvailable() < (qint64)(byteCount))
    {
        if (!socket.waitForReadyRead(timeout))
        {
            error((("Waiting for data: ") + socket.errorString()).toLatin1().data());
            return;
        }
        OCTAVE_QUIT;
    }


    octave_idx_type valueCount = cellCornerValues.length();

    octave_stdout << "GetCellCorners - after wait for data" << std::endl;

    double* internalMatrixData = cellCornerValues.fortran_vec();


#if 1
    double val;
    for (octave_idx_type i = 0; i < valueCount; i++)
    {
        socketStream >> internalMatrixData[i];
    }
#else
    quint64 bytesRead = 0;
    bytesRead = socket.read((char*)(internalMatrixData), byteCount);

    if (byteCount != bytesRead)
    {
        error("Could not read binary double data properly from socket");
        octave_stdout << "Cell count: " << cellCount << std::endl;
    }

#endif

    return;
}



DEFUN_DLD (riGetCellCorners, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetCellCorners([CaseId], GridIndex )\n"
           "\n"
           "This function returns the UTM coordinates (X, Y, Z) of the 8 corners of all the cells in the grid.\n"
           "If the CaseId is not defined, ResInsight’s Current Case is used.\n"
           )
{
    int nargin = args.length ();
    if (nargin > 2)
    {
        error("riGetCellCorners: Too many arguments. CaseId is optional input argument.\n");
        print_usage();
    }
    else if (nargout < 1)
    {
        error("riGetCellCorners: Missing output argument.\n");
        print_usage();
    }
    else
    {
        NDArray cellCornerValues;

        qint32 caseId = -1;
        quint32 gridIndex = 0;

        if (nargin == 1)
        {
            gridIndex = args(0).uint_value();
        }
        else if (nargin == 2)
        {
            unsigned int argCaseId = args(0).uint_value();
            caseId = argCaseId;

            gridIndex = args(1).uint_value();
        }

        getCellCorners(cellCornerValues, "127.0.0.1", 40001, caseId, gridIndex);

        return octave_value(cellCornerValues);
    }

    return octave_value();
}

