#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"


void getCellCenters(NDArray& cellCenterValues, const QString &hostName, quint16 port, const qint32& caseId, const quint32& gridIndex)
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

    QString command = QString("GetCellCenters %1 %2").arg(caseId).arg(gridIndex);
    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)(2 * sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(timeout))
        {
            error((("Wating for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

    // Read timestep count and blocksize

    quint64 cellCount;
    quint64 byteCount;

    socketStream >> cellCount;
    socketStream >> byteCount;

    dim_vector dv (1, 1);
    dv(0) = 3;
    dv(1) = cellCount;

    cellCenterValues.resize(dv);

    if (!(byteCount && cellCount))
    {
        error ("Could not find the requested data in ResInsight");
        return;
    }

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

    quint64 bytesRead = 0;
    double* internalMatrixData = cellCenterValues.fortran_vec();
    bytesRead = socket.read((char*)(internalMatrixData), byteCount);

    if (byteCount != bytesRead)
    {
        error("Could not read binary double data properly from socket");
        octave_stdout << "Cell count: " << cellCount << std::endl;
    }

    return;
}



DEFUN_DLD (riGetCellCenters, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetActiveCellInfo([CaseId], GridIndex )\n"
           "\n"
           "This function returns the UTM coordinates (X, Y, Z) of the center point of all the cells in the grid.\n"
           "If the CaseId is not defined, ResInsight’s Current Case is used.\n"
           )
{
    int nargin = args.length ();
    if (nargin > 2)
    {
        error("riGetCellCenters: Too many arguments. CaseId is optional input argument.\n");
        print_usage();
    }
    else if (nargout < 1)
    {
        error("riGetCellCenters: Missing output argument.\n");
        print_usage();
    }
    else
    {
        NDArray cellCenterValues;

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

        getCellCenters(cellCenterValues, "127.0.0.1", 40001, caseId, gridIndex);

        return octave_value(cellCenterValues);
    }

    return octave_value();
}

