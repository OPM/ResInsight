#include <QtNetwork>
#include <octave/oct.h>

#include "riSettings.h"

void getTimeStepDates(  std::vector<double>& decimalDays,
                        const qint64& caseId,
                        const QString& hostName,
                        quint16 port)
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

    QString command = QString("GetTimeStepDays %1").arg(caseId);
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

    while (socket.bytesAvailable() < (int)(byteCount))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error("Waiting for data: %s",socket.errorString().toLatin1().data());
            return;
        }
        OCTAVE_QUIT;
    }

    quint64 timeStepCount;
    socketStream >> timeStepCount;

    octave_stdout << "byte count: " << byteCount << ", Timesteps: " << timeStepCount << std::endl;

    for (size_t i = 0; i < timeStepCount; i++)
    {
        double doubleValue;

        socketStream >> doubleValue;
        decimalDays.push_back(doubleValue);
    }

    return;
}



DEFUN_DLD (riGetTimeStepDays, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetTimeStepDays()\n"
           "\n"
           "This function returns the time from the simulation start\n"
           "as decimal days for all the time steps as a Vector of doubles.\n"
           "If the CaseId is not defined, ResInsight’s Current Case is used.\n"
           )
{
    int nargin = args.length ();
    if (nargin > 1)
    {
        error("riGetTimeStepDays: Too many arguments, this function takes one optional argument.\n");
        print_usage();
    }
    else if (nargout != 1)
    {
        error("riGetTimeStepDays: Wrong number of output arguments, this function requires one output argument.\n");
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

        std::vector<double> decimalDays;

        getTimeStepDates(decimalDays, caseId, "127.0.0.1", 40001);

        dim_vector dv(2, 1);
        dv(0) = decimalDays.size();
        dv(1) = 1;
        NDArray oct_decimalDays(dv);

        for (size_t i = 0; i < decimalDays.size(); i++)
        {
            oct_decimalDays(i) = decimalDays[i];
        }

        return octave_value(oct_decimalDays);
    }

    return octave_value();
}

