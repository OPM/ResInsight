#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"

void getMainGridDimensions(int32NDArray& gridDimensions, const QString &hostName, quint16 port, QString caseName)
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

    QString command("GetMainGridDimensions ");
    command += caseName;
    QByteArray cmdBytes = command.toLatin1();

    QDataStream socketStream(&socket);
    socketStream.setVersion(QDataStream::Qt_4_0);

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)(3*sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error("Waiting for header: %s",socket.errorString().toLatin1().data());
            return;
        }
    }

    // Read timestep count and blocksize

    quint64 iCount;
    quint64 jCount;
    quint64 kCount;

    socketStream >> iCount;
    socketStream >> jCount;
    socketStream >> kCount;

    dim_vector dv (1, 1);
    dv(0) = 3;

    gridDimensions.resize(dv);
    gridDimensions(0) = iCount;
    gridDimensions(1) = jCount;
    gridDimensions(2) = kCount;


    QString tmp = QString("riGetMainGridDimensions : Read main grid dimensions");
    if (caseName.isEmpty())
    {
        tmp += QString(" from active case.");
    }
    else
    {
        tmp += QString(" from %1.").arg(caseName);
    }
    octave_stdout << tmp.toStdString() << " Dimensions: " << iCount << ", " << jCount << ", " << kCount << std::endl;

    return;
}



DEFUN_DLD (riGetMainGridDimensions, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetMainGridDimensions( [CaseName/CaseIndex])\n"
           "\n"
           "Returns a vector of size 3: [ICount, JCount, KCount] \n"
           "Containing the dimensions of the main grid in the requested case.\n"
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
            getMainGridDimensions(propertyFrames, "127.0.0.1", 40001, args(0).char_matrix_value().row_as_string(0).c_str());
        else
            getMainGridDimensions(propertyFrames, "127.0.0.1", 40001, "");

        return octave_value(propertyFrames);
    }

    return octave_value_list ();
}

