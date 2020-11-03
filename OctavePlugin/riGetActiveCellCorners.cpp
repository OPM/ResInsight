#include <QtNetwork>
#include <QStringList>

#include <octave/oct.h>

#include "riSettings.h"
#include "RiaSocketDataTransfer.cpp"  // NB! Include cpp-file to avoid linking of additional file in oct-compile configuration


void getActiveCellCorners(NDArray& cellCornerValues, const QString &hostName, quint16 port, const qint32& caseId, const QString& porosityModel)
{
    QString serverName = hostName;
    quint16 serverPort = port;

    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(riOctavePlugin::connectTimeOutMilliSecs))
    {
        error("Connection: %s",socket.errorString().toLatin1().data());
        return;
    }

    // Create command and send it:

    QString command = QString("GetActiveCellCorners %1 %2").arg(caseId).arg(porosityModel);
    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)(2 * sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error("Waiting for header: %s",socket.errorString().toLatin1().data());
            return;
        }
    }

    // Read timestep count and blocksize

    quint64 activeCellCount;
    quint64 byteCount;

    socketStream >> activeCellCount;
    socketStream >> byteCount;

    if (!(byteCount && activeCellCount))
    {
        error ("Could not find the requested data in ResInsight");
        return;
    }

    dim_vector dv;
    dv.resize(3);
    dv(0) = activeCellCount;
    dv(1) = 8;
    dv(2) = 3;
    cellCornerValues.resize(dv);

    double* internalMatrixData = cellCornerValues.fortran_vec();
    QStringList errorMessages;
    if (!RiaSocketDataTransfer::readBlockDataFromSocket(&socket, (char*)(internalMatrixData), byteCount, errorMessages))
    {
        for (int i = 0; i < errorMessages.size(); i++)
        {
            error("%s",errorMessages[i].toLatin1().data());
        }

        OCTAVE_QUIT;
    }

    return;
}



DEFUN_DLD (riGetActiveCellCorners, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetActiveCellCorners([CaseId], [PorosityModel = “Matrix”|”Fracture”] )\n"
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

    if (nargin > 2)
    {
        error("riGetActiveCellCorners: Too many arguments.\n");
        print_usage();
        return octave_value_list ();
    }

    qint32 caseId = -1;
    std::string porosityModel = "Matrix";

    if (nargin == 1)
    {
        if (riOctavePlugin::isOctaveValueNumeric(args(0)))
        {
            caseId = args(0).uint_value();
        }
        else
        {
            porosityModel = args(0).string_value();
        }
    }
    else if (nargin == 2)
    {
        if (riOctavePlugin::isOctaveValueNumeric(args(0)))
        {
            caseId        = args(0).uint_value();
            porosityModel = args(1).string_value();
        }
        else
        {
            caseId        = args(1).uint_value();
            porosityModel = args(0).string_value();
        }
    }

    if (porosityModel != "Matrix" && porosityModel != "Fracture")
    {
        error("riGetActiveCellProperty: The value for \"PorosityModel\" is unknown. Please use either \"Matrix\" or \"Fracture\"\n");
        print_usage();
        return octave_value_list ();
    }

    NDArray cellCornerValues;
    getActiveCellCorners(cellCornerValues, "127.0.0.1", 40001, caseId, porosityModel.c_str());

    return octave_value(cellCornerValues);
}

