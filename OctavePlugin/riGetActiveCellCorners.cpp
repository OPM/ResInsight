#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"


void getActiveCellCorners(NDArray& cellCornerValues, const QString &hostName, quint16 port, const qint32& caseId, const quint32& gridIndex, const QString& porosityModel)
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

    QString command = QString("GetActiveCellCorners %1 %2 %3").arg(caseId).arg(gridIndex).arg(porosityModel);
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

    quint64 coordCount;
    quint64 byteCount;

    socketStream >> coordCount;
    socketStream >> byteCount;

    dim_vector dv (1, 1);
    dv(0) = 3;
    dv(1) = coordCount;

    cellCornerValues.resize(dv);

    if (!(byteCount && coordCount))
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
    double* internalMatrixData = cellCornerValues.fortran_vec();
    bytesRead = socket.read((char*)(internalMatrixData), byteCount);

    if (byteCount != bytesRead)
    {
        error("Could not read binary double data properly from socket");
        octave_stdout << "Active cell count: " << coordCount << std::endl;
    }

    return;
}



DEFUN_DLD (riGetActiveCellCorners, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetActiveCellCorners([CaseId], GridIndex, [PorosityModel = “Matrix”|”Fracture”] )\n"
           "\n"
           "This function returns the UTM coordinates (X, Y, Z) of the 8 corners of each of the active cells.\n"
           "If the CaseId is not defined, ResInsight’s Current Case is used.\n"
           )
{

    if (nargout < 1)
    {
        error("riGetActiveCellCorners: Missing output argument.\n");
        print_usage();
        return octave_value_list ();
    }

    int nargin = args.length ();
    if (nargin < 1)
    {
        error("riGetActiveCellCorners: Too few arguments. The grid index argument is required.\n");
        print_usage();
        return octave_value_list ();
    }

    if (nargin > 3)
    {
        error("riGetActiveCellCorners: Too many arguments.\n");
        print_usage();
        return octave_value_list ();
    }

    qint32 caseId = -1;
    quint32 gridIndex = 0;
    std::string porosityModel = "Matrix";

    if (nargin == 1)
    {
        gridIndex = args(0).uint_value();
    }
    else if (nargin == 2)
    {
        if (args(0).is_numeric_type() && args(1).is_numeric_type())
        {
            caseId = args(0).uint_value();
            gridIndex = args(1).uint_value();
        }
        else
        {
            gridIndex     = args(0).uint_value();
            porosityModel = args(1).string_value();
        }
    }
    else if (nargin == 3)
    {
        caseId        = args(0).uint_value();
        gridIndex     = args(1).uint_value();
        porosityModel = args(2).string_value();
    }

    if (porosityModel != "Matrix" && porosityModel != "Fracture")
    {
        error("riGetActiveCellProperty: The value for \"PorosityModel\" is unknown. Please use either \"Matrix\" or \"Fracture\"\n");
        print_usage();
        return octave_value_list ();
    }

    NDArray cellCornerValues;
    getActiveCellCorners(cellCornerValues, "127.0.0.1", 40001, caseId, gridIndex, porosityModel.c_str());

    return octave_value(cellCornerValues);
}

