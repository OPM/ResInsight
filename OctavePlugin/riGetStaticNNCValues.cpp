#include <QtNetwork>
#include <QStringList>

#include <octave/oct.h>

#include "riSettings.h"

#include "RiaSocketDataTransfer.cpp"  // NB! Include cpp-file to avoid linking of additional file in oct-compile configuration

void getStaticNNCValues(std::vector<double>& propertyValues, const QString &serverName, quint16 serverPort,
                         const qint64& caseId, QString propertyName)
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
    command += "GetStaticNNCValues " + QString::number(caseId) + " " + propertyName;

    QByteArray cmdBytes = command.toLatin1();

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Get response. First wait for the header

    while (socket.bytesAvailable() < (int)sizeof(quint64))
    {
        if (!socket.waitForReadyRead(riOctavePlugin::longTimeOutMilliSecs))
        {
            error((("Waiting for header: ") + socket.errorString()).toLatin1().data());
            return;
        }
    }

    // Read connection count and timestep count
    quint64 connectionCount;

    socketStream >> connectionCount;

    if (!(connectionCount))
    {
        error ("Could not find the requested data in ResInsight");
        return;
    }

    propertyValues.reserve(connectionCount);

    for (size_t i = 0; i < connectionCount; ++i)
    {
        double val;
        socketStream >> val;
        propertyValues.push_back(val);
    }

    QString tmp = QString("riGetStaticNNCValues : Read %1").arg(propertyName);

    if (caseId < 0)
    {
        tmp += QString(" from current case.");
    }
    else
    {
        tmp += QString(" from case with Id: %1.").arg(caseId);
    }
    octave_stdout << tmp.toStdString() << " Connections: " << connectionCount << std::endl;

    return;
}


DEFUN_DLD (riGetStaticNNCValues, args, nargout,
           "Usage:\n"
           "\n"
           "   riGetStaticNNCValues([CaseId], PropertyName)\n"
           "\n"
           "This function returns a vector with the static NNC values for each connection.\n"
           "\n"
           "See riGetNNCConnections for information about each individual connection.\n"
           "If the CaseId is not defined, ResInsight's Current Case is used.\n"
           )
{
    int nargin = args.length ();
    if (nargin < 1)
    {
        error("riGetStaticNNCValues: Too few arguments. The name of the property requested is necessary.\n");
        print_usage();
        return octave_value();
    }
    else if (nargin > 2)
    {
        error("riGetStaticNNCValues: Too many arguments.\n");
        print_usage();
        return octave_value();
    }
    else if (nargout < 1)
    {
        error("riGetStaticNNCValues: Missing output argument.\n");
        print_usage();
        return octave_value();
    }

    std::vector<int> argIndices;
    argIndices.push_back(0);
    argIndices.push_back(1);

    // Check if we have a CaseId:
    if (!args(argIndices[0]).is_numeric_type())
    {
        argIndices[0] = -1;
        for (size_t aIdx = 1; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    std::vector<double> propertyValues;
    qint32 caseId = -1;
    std::string propertyName;

    if (argIndices[0] >= 0) caseId       = args(argIndices[0]).int_value();
    if (argIndices[1] >= 0) propertyName = args(argIndices[1]).char_matrix_value().row_as_string(0);

    getStaticNNCValues(propertyValues, "127.0.0.1", 40001, caseId, propertyName.c_str());

    dim_vector dv(2, 1);
    dv(0) = propertyValues.size();
    dv(1) = 1;
    NDArray oct_propertyValues(dv);

    for (size_t i = 0; i < propertyValues.size(); ++i)
    {
        oct_propertyValues(i) = propertyValues[i];
    }

    return octave_value(oct_propertyValues);
}
