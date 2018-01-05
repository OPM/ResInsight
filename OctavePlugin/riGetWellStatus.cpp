#include <QtNetwork>
#include <octave/oct.h>
#include <octave/oct-map.h>

#include "riSettings.h"

void getWellStatus(std::vector<QString>& wellTypes, std::vector<int>& wellStatuses, const QString &hostName, quint16 port, 
                        const qint64& caseId, const QString& wellName, const int32NDArray& requestedTimeSteps)
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

    QString command;
    command += QString("GetWellStatus") + " " + QString::number(caseId) + " " + wellName;

    for (int i = 0; i < requestedTimeSteps.length(); ++i)
    {
        if (i == 0) command += " ";
        command += QString::number(static_cast<int>(requestedTimeSteps.elem(i)) - 1); // To make the index 0-based
        if (i != requestedTimeSteps.length() -1) command += " ";
    }

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
            error((("Waiting for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

    quint64 byteCount;
    socketStream >> byteCount;

    while (socket.bytesAvailable() < (int)(byteCount))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error((("Waiting for data: ") + socket.errorString()).toLatin1().data());
            return;
        }
        OCTAVE_QUIT;
    }

    quint64 timeStepCount;
    socketStream >> timeStepCount;

    QString wellType;
    qint32 wellStatus;

    for (size_t i = 0; i < timeStepCount; i++)
    {
        socketStream >> wellType;
        socketStream >> wellStatus;

        wellTypes.push_back(wellType);
        wellStatuses.push_back(wellStatus);
    }

    return;
}



DEFUN_DLD (riGetWellStatus, args, nargout,
    "Usage:\n"
    "\n"
    "   riGetWellStatus ([CaseId], WellName, [RequestedTimeSteps]) \n"
    "\n"
    "This function returns the status information for a specified well for each\n"
    "requested time step as a vector of Structures. \n"
    "The Structure is defined as:\n"
    "WellStatus { \n"
    "    WellType	= string	 # \"Producer\", \"OilInjector\", \"WaterInjector\", \"GasInjector\", \"NotDefined\" \n"
    "    WellStatus	= int    	 # is either 0 or 1, meaning the well is shut or open respectively.\n"
    "}\n"
    "If the CaseId is not defined, ResInsight’s Current Case is used.\n"

    )
{
    if (nargout != 1)
    {
        error("riGetWellStatus: Wrong number of output arguments, this function requires one output argument.\n");
        print_usage();
        return octave_value();
    }

    int nargin = args.length ();
    if (nargin < 1)
    {
        error("riGetWellStatus: Too few arguments, this function needs at least the well name as input.\n");
        print_usage();
        return octave_value();
    }
 
    if (nargin > 3)
    {
        error("riGetWellStatus: Too many arguments, this function takes at most three arguments.\n");
        print_usage();
        return octave_value();
    }

    std::vector<int> argIndices;
    argIndices.push_back(0); // caseId
    argIndices.push_back(1); // WellName
    argIndices.push_back(2); // TimeSteps

    // Check if we do not have a CaseId:
    if (args(argIndices[0]).is_string()) // Check if first argument is a text. If it is, the caseId is missing
    {
        argIndices[0] = -1;
        for (size_t aIdx = 1; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have a Requested TimeSteps
    int lastArgumentIndex = argIndices[2] ;
    if (!(nargin > argIndices[2] && (args(argIndices[2]).is_matrix_type() || args(argIndices[2]).is_numeric_type())))
    {
        argIndices[2] = -1;
    }

    // Check if we have more arguments than we should
    if (nargin > lastArgumentIndex + 1)
    {
        error("riGetWellStatus: Unexpected argument at the end.\n");
        print_usage();
        return octave_value_list ();
    }

    // Setup the argument list

    NDArray propertyFrames;
    int caseId = -1;
    std::string wellName = "UNDEFINED";
    int32NDArray requestedTimeSteps;

    if (argIndices[0] >= 0) caseId              = args(argIndices[0]).int_value();
    if (argIndices[1] >= 0) wellName            = args(argIndices[1]).char_matrix_value().row_as_string(0);
    if (argIndices[2] >= 0) requestedTimeSteps  = args(argIndices[2]).int32_array_value();

    if (wellName == "UNDEFINED")
    {
        error("riGetWellStatus: The argument must be a text containing the well name.\n");
        print_usage();
        return octave_value();
    }

    std::vector<QString> wellType;
    std::vector<int> wellStatus;

    getWellStatus(wellType, wellStatus, "127.0.0.1", 40001, caseId, QString::fromStdString(wellName), requestedTimeSteps);

    size_t caseCount = wellType.size();

    if (wellType.size() != wellStatus.size() )
    {
        error("riGetWellStatus: Inconsistent data received from ResInsight.\n");
        return octave_value();
    }
    else
    {
        // Create cells with N items for each field in the data structure

        Cell cellValuesB(caseCount, 1);
        Cell cellValuesC(caseCount, 1);

        for (size_t i = 0; i < caseCount; i++)
        {
            cellValuesB(i) = wellType[i].toLatin1().data();
            cellValuesC(i) = wellStatus[i];
        }

        // Build a map between the field name and field cell values

        octave_map m;

        m.assign(riOctavePlugin::wellStatus_WellType,    cellValuesB);
        m.assign(riOctavePlugin::wellStatus_WellStatus,  cellValuesC);

        return octave_value(m);
    }
   
}

