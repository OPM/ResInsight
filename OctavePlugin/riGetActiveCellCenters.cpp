#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"


void getActiveCellCenters(NDArray& cellCenterValues, const QString &hostName, quint16 port, const qint32& caseId, const quint32& gridIndex, const QString& porosityModel)
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

    QString command = QString("GetActiveCellCenters %1 %2 %3").arg(caseId).arg(gridIndex).arg(porosityModel);
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

    quint64 activeCellCount;
    quint64 byteCount;

    socketStream >> activeCellCount;
    socketStream >> byteCount;

    dim_vector dv (1, 1);
    dv(0) = 3;
    dv(1) = activeCellCount;

    cellCenterValues.resize(dv);

    if (!(byteCount && activeCellCount))
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
        octave_stdout << "Active cell count: " << activeCellCount << std::endl;
    }

    return;
}



DEFUN_DLD (riGetActiveCellCenters, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetActiveCellCenters([CaseId], GridIndex, [PorosityModel = “Matrix”|”Fracture”] )\n"
           "\n"
           "This function returns the UTM coordinates (X, Y, Z) of the center point of all the cells in the grid.\n"
           "If the CaseId is not defined, ResInsight’s Current Case is used.\n"
           )
{

    if (nargout < 1)
    {
        error("riGetActiveCellCenters: Missing output argument.\n");
        print_usage();
        return octave_value_list ();
    }

    int nargin = args.length ();
    if (nargin < 1)
    {
        error("riGetActiveCellCenters: Too few arguments. The grid index argument is required.\n");
        print_usage();
        return octave_value_list ();
    }

    if (nargin > 3)
    {
        error("riGetActiveCellCenters: Too many arguments.\n");
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

    NDArray cellCenterValues;
    getActiveCellCenters(cellCenterValues, "127.0.0.1", 40001, caseId, gridIndex, porosityModel.c_str());

    return octave_value(cellCenterValues);
}

