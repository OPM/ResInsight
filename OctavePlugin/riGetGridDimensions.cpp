#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"


void getGridDimensions(int32NDArray& gridDimensions, const QString &hostName, quint16 port, const qint64& caseId)
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

    QString command = QString("GetGridDimensions %1").arg(caseId);
    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)(sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error("Waiting for header: %s",socket.errorString().toLatin1().data());
            return;
        }
    }

    quint64 byteCount;
    socketStream >> byteCount;

    quint64 gridCount = byteCount / (3 * sizeof(quint64));

    dim_vector dv (1, 1);
    dv(0) = gridCount;
    dv(1) = 3;

    gridDimensions.resize(dv);

    for (size_t i = 0; i < gridCount; i++)
    {
        quint64 iCount;
        quint64 jCount;
        quint64 kCount;

        socketStream >> iCount;
        socketStream >> jCount;
        socketStream >> kCount;

        gridDimensions(i, 0) = iCount;
        gridDimensions(i, 1) = jCount;
        gridDimensions(i, 2) = kCount;
    }

    QString tmp = QString("riGetGridDimensions : Read grid dimensions");
    if (caseId < 0)
    {
        tmp += QString(" from current case.");
    }
    else
    {
        tmp += QString(" from caseID: %1.").arg(caseId);
    }

    octave_stdout << tmp.toStdString() << std::endl;

    return;
}



DEFUN_DLD (riGetGridDimensions, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetGridDimensions([CaseId])\n"
           "\n"
           "This function returns a two dimensional matrix: One row for each grid, starting with the main grid.\n"
           "NOTE: This means that the “normal” GridIndices where 0 means Main Grid does not work directly with this matrix. You have to add 1.\n"
           "The columns contain the following information:\n"
           "[NI, NJ, NK]: I, J, K dimensions of the grid.\n"
           "If the CaseId is not defined, ResInsight’s Current Case is used.\n"
           )
{
    int nargin = args.length ();
    if (nargin > 1)
    {
        error("riGetGridDimensions: Too many arguments. Only the name or index of the case is valid input.\n");
        print_usage();
    }
    else if (nargout < 1)
    {
        error("riGetGridDimensions: Missing output argument.\n");
        print_usage();
    }
    else
    {
        qint64 caseId = -1;
        if (nargin > 0)
        {
            if (riOctavePlugin::isOctaveValueNumeric(args(0)))
            {
                unsigned int argCaseId = args(0).uint_value();
                caseId = argCaseId;
            }
        }

        int32NDArray gridDimensions;
        getGridDimensions(gridDimensions, "127.0.0.1", 40001, caseId);

        return octave_value(gridDimensions);
    }

    return octave_value_list ();
}

