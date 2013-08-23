#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"


void getCellCenters(NDArray& cellCenterValues, const QString &hostName, quint16 port, const qint32& caseId, const quint32& gridIndex)
{
    QString serverName = hostName;
    quint16 serverPort = port;

    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(riOctavePlugin::connectTimeOutMilliSecs))
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

    while (socket.bytesAvailable() < (int)(5 * sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::shortTimeOutMilliSecs))
        {
            error((("Waiting for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

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

    if (!(byteCount && cellCount))
    {
        error ("Could not find the requested data in ResInsight");
        return;
    }

    dim_vector dv;
    dv.resize(4);
    dv(0) = cellCountI;
    dv(1) = cellCountJ;
    dv(2) = cellCountK;
    dv(3) = 3;
    cellCenterValues.resize(dv);

    while (socket.bytesAvailable() < (qint64)(byteCount))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error((("Waiting for data: ") + socket.errorString()).toLatin1().data());
            return;
        }
        OCTAVE_QUIT;
    }

    //octave_stdout << " riGetCellCenters : I = " << cellCountI <<" J = " << cellCountJ << " K = " << cellCountK  << std::endl;
    //octave_stdout << " riGetCellCenters : numDoubles = " << valueCount << std::endl;

    double* internalMatrixData = cellCenterValues.fortran_vec();

#if 0
    octave_idx_type valueCount = cellCenterValues.length();
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



DEFUN_DLD (riGetCellCenters, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetCellCenters([CaseId], GridIndex )\n"
           "\n"
           "This function returns the UTM coordinates (X, Y, Z) of the center point of all the cells in the grid.\n"
           "If the CaseId is not defined, ResInsight�s Current Case is used.\n"
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

