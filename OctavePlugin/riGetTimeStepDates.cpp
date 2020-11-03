#include <QtNetwork>
#include <octave/oct.h>
#include <octave/oct-map.h>

#include "riSettings.h"

void getTimeStepDates(  std::vector<qint32>& yearValues,
                        std::vector<qint32>& monthValues,
                        std::vector<qint32>& dayValues,
                        std::vector<qint32>& hourValues,
                        std::vector<qint32>& minuteValues,
                        std::vector<qint32>& secondValues,
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

    QString command = QString("GetTimeStepDates %1").arg(caseId);
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

    for (size_t i = 0; i < timeStepCount; i++)
    {
        qint32 intValue;

        socketStream >> intValue;
        yearValues.push_back(intValue);

        socketStream >> intValue;
        monthValues.push_back(intValue);

        socketStream >> intValue;
        dayValues.push_back(intValue);

        socketStream >> intValue;
        hourValues.push_back(intValue);

        socketStream >> intValue;
        minuteValues.push_back(intValue);

        socketStream >> intValue;
        secondValues.push_back(intValue);
    }

    return;
}



DEFUN_DLD (riGetTimeStepDates, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetTimeStepDates()\n"
           "\n"
           "This function returns the date information for each of the time steps in the case as a Vector of Structures.\n"
           "The Structure is defined as:\n"
            "TimeStepDate = {\n"
            "    Year = int # The year eg. 2013\n"
            "    Month = int # The month. Eg. 12\n"
            "    Day = int # The day in the month. Eg. 24\n"
            "    Hour = int # The hour of the day. Eg. 17\n"
            "    Minute = int # The minute in the hour. Eg. 55\n"
            "    Second = int # The second within the minute. Eg. 30\n"
            "}\n"
            "If the CaseId is not defined, ResInsight’s Current Case is used.\n"
           )
{
    int nargin = args.length ();
    if (nargin > 1)
    {
        error("riGetTimeStepDates: Too many arguments, this function takes one optional argument.\n");
        print_usage();
    }
    else if (nargout != 1)
    {
        error("riGetTimeStepDates: Wrong number of output arguments, this function requires one output argument.\n");
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

        std::vector<qint32> yearValues;
        std::vector<qint32> monthValues;
        std::vector<qint32> dayValues;
        std::vector<qint32> hourValues;
        std::vector<qint32> minuteValues;
        std::vector<qint32> secondValues;

        getTimeStepDates(yearValues, monthValues, dayValues, hourValues, minuteValues, secondValues, caseId, "127.0.0.1", 40001);

        size_t timeStepDateCount = yearValues.size();

        // Create cells with N items for each field in the data structure
            
        Cell cellValuesA(timeStepDateCount, 1);
        Cell cellValuesB(timeStepDateCount, 1);
        Cell cellValuesC(timeStepDateCount, 1);
        Cell cellValuesD(timeStepDateCount, 1);
        Cell cellValuesE(timeStepDateCount, 1);
        Cell cellValuesF(timeStepDateCount, 1);

        for (size_t i = 0; i < timeStepDateCount; i++)
        {
            cellValuesA(i) = yearValues[i];
            cellValuesB(i) = monthValues[i];
            cellValuesC(i) = dayValues[i];
            cellValuesD(i) = hourValues[i];
            cellValuesE(i) = minuteValues[i];
            cellValuesF(i) = secondValues[i];
        }

        // Build a map between the field name and field cell values
            
        octave_map m;

        m.assign(riOctavePlugin::timeStepDate_Year,     cellValuesA);
        m.assign(riOctavePlugin::timeStepDate_Month,    cellValuesB);
        m.assign(riOctavePlugin::timeStepDate_Day,      cellValuesC);
        m.assign(riOctavePlugin::timeStepDate_Hour,     cellValuesD);
        m.assign(riOctavePlugin::timeStepDate_Minute,   cellValuesE);
        m.assign(riOctavePlugin::timeStepDate_Second,   cellValuesF);

        return octave_value(m);
    }

    return octave_value();
}

