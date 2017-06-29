#include <QtNetwork>
#include <QStringList>

#include <octave/oct.h>

#include "riSettings.h"

#include "RiaSocketDataTransfer.cpp"  // NB! Include cpp-file to avoid linking of additional file in oct-compile configuration

void getDynamicNNCValues(Matrix& propertyFrames, const QString &serverName, quint16 serverPort,
                         const qint64& caseId, QString propertyName, const int32NDArray& requestedTimeSteps)
{
    QTcpSocket socket;
    socket.connectToHost(serverName, serverPort);

    if (!socket.waitForConnected(riOctavePlugin::connectTimeOutMilliSecs))
    {
        error((("Connection: ") + socket.errorString()).toLatin1().data());
        return;
    }

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    // Create command as a string with arguments , and send it:
    QString command;
    command += "GetDynamicNNCValues " + QString::number(caseId) + " " + propertyName;

    for (int i = 0; i < requestedTimeSteps.length(); ++i)
    {
        if (i == 0) command += " ";
        command += QString::number(static_cast<int>(requestedTimeSteps.elem(i)) - 1); // To make the index 0-based
        if (i != requestedTimeSteps.length() -1) command += " ";
    }

    QByteArray cmdBytes = command.toLatin1();

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)(2*sizeof(quint64)))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error((("Waiting for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

    // Read connection count and timestep count
    quint64 connectionCount;
    quint64 timestepCount;

    socketStream >> connectionCount;
    socketStream >> timestepCount;

    propertyFrames.resize(connectionCount, timestepCount);

    if (!(connectionCount && timestepCount))
    {
        error ("Could not find the requested data in ResInsight");
        return;
    }

    quint64 totalByteCount = timestepCount * connectionCount * sizeof(double);

    double* internalMatrixData = propertyFrames.fortran_vec();
    QStringList errorMessages;
    if (!RiaSocketDataTransfer::readBlockDataFromSocket(&socket, (char*)(internalMatrixData), totalByteCount, errorMessages))
    {
        for (int i = 0; i < errorMessages.size(); i++)
        {
            error(errorMessages[i].toLatin1().data());
        }

        return;
    }

    QString tmp = QString("riGetDynamicNNCValues : Read %1").arg(propertyName);

    if (caseId < 0)
    {
        tmp += QString(" from current case.");
    }
    else
    {
        tmp += QString(" from case with Id: %1.").arg(caseId);
    }
    octave_stdout << tmp.toStdString() << " Connections: " << connectionCount << ", Time steps : " << timestepCount << std::endl;

    return;
}


DEFUN_DLD (riGetDynamicNNCValues, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetDynamicNNCValues([CaseId], PropertyName, [RequestedTimeSteps])\n"
           "\n"
           "This function retrieves the dynamic NNC values for each connection for the requested time steps."
           )
{
    int nargin = args.length ();
    if (nargin < 1)
    {
        error("riGetDynamicNNCValues: Too few arguments. The name of the property requested is necessary.\n");
        print_usage();
        return octave_value_list();
    }
    else if (nargin > 3)
    {
        error("riGetDynamicNNCValues: Too many arguments.\n");
        print_usage();
        return octave_value_list();
    }
    else if (nargout < 1)
    {
        error("riGetDynamicNNCValues: Missing output argument.\n");
        print_usage();
        return octave_value_list();
    }

    std::vector<int> argIndices;
    argIndices.push_back(0);
    argIndices.push_back(1);
    argIndices.push_back(2);

    // Check if we have a CaseId:
    if (!args(argIndices[0]).is_numeric_type())
    {
        argIndices[0] = -1;
        for (size_t aIdx = 1; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have a Requested TimeSteps
    if (!(nargin > argIndices[2] && args(argIndices[2]).is_matrix_type()))
    {
        argIndices[2] = -1;
    }

    Matrix propertyFrames;
    qint32 caseId = -1;
    int32NDArray requestedTimeSteps;
    std::string propertyName;

    if (argIndices[0] >= 0) caseId             = args(argIndices[0]).int_value();
    if (argIndices[1] >= 0) propertyName       = args(argIndices[1]).char_matrix_value().row_as_string(0);
    if (argIndices[2] >= 0) requestedTimeSteps = args(argIndices[2]).int32_array_value();

    getDynamicNNCValues(propertyFrames, "127.0.0.1", 40001, caseId, propertyName.c_str(), requestedTimeSteps);

    return octave_value(propertyFrames);
}
