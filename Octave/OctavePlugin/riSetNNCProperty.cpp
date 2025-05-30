#include <QtNetwork>
#include <QStringList>

#include <octave/oct.h>

#include "riSettings.h"
#include "RiaSocketDataTransfer.cpp"  // NB! Include cpp-file to avoid linking of additional file in oct-compile configuration

#ifdef WIN32
#include <Windows.h>
#endif //WIN32


void setNNCProperty(const Matrix& propertyFrames, const QString &hostName, quint16 port,
                    const qint64& caseId, QString propertyName, const int32NDArray& requestedTimeSteps)
{
    QTcpSocket socket;
    socket.connectToHost(hostName, port);

    if (!socket.waitForConnected(riOctavePlugin::connectTimeOutMilliSecs))
    {
        error("Connection: %s",socket.errorString().toLatin1().data());
        return;
    }

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    // Create command as a string with arguments , and send it:

    QString command;
    command += "SetNNCProperty " + QString::number(caseId) + " " + propertyName;

    for (int i = 0; i < requestedTimeSteps.numel(); ++i)
    {
        if (i == 0) command += " ";
        command += QString::number(static_cast<int>(requestedTimeSteps.elem(i)) - 1); // To make the index 0-based
        if (i != requestedTimeSteps.numel() -1) command += " ";
    }

    QByteArray cmdBytes = command.toLatin1();

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Write property data header

    dim_vector mxDims = propertyFrames.dims();

    qint64 connectionCount = mxDims.elem(0);
    qint64 timeStepCount = mxDims.elem(1);
    qint64 timeStepByteCount = connectionCount * sizeof(double);

    socketStream << (qint64)(timeStepCount);
    socketStream << (qint64)timeStepByteCount;

    const double* internalData = propertyFrames.fortran_vec();

    QStringList errorMessages;
    if (!RiaSocketDataTransfer::writeBlockDataToSocket(&socket, (const char *)internalData, timeStepByteCount*timeStepCount, errorMessages))
    {
        for (int i = 0; i < errorMessages.size(); i++)
        {
            octave_stdout << errorMessages[i].toStdString();
        }

        return;
    }

    QString tmp = QString("riSetNNCProperty : Wrote %1").arg(propertyName);

    if (caseId == -1)
    {
        tmp += QString(" to current case.");
    }
    else
    {
        tmp += QString(" to case with Id = %1.").arg(caseId);
    }
    octave_stdout << tmp.toStdString() << " NNC Connections : " << connectionCount << " Time steps : " << timeStepCount << std::endl;

    while(socket.bytesToWrite() && socket.state() == QAbstractSocket::ConnectedState)
    {
        // octave_stdout << "Bytes to write: " << socket.bytesToWrite() << std::endl;
        socket.waitForBytesWritten(riOctavePlugin::shortTimeOutMilliSecs);
        OCTAVE_QUIT;
    }

    //octave_stdout << "    Socket write completed" << std::endl;

    if (socket.bytesToWrite() && socket.state() != QAbstractSocket::ConnectedState)
    {
        error("riSetNNCProperty : ResInsight refused to accept the data. Maybe the dimensions or porosity model is wrong");
    }

#ifdef WIN32
    // TODO: Due to synchronization issues seen on Windows 10, it is required to do a sleep here to be able to catch disconnect
    // signals from the socket. No sleep causes the server to hang.

    Sleep(100);

#endif //WIN32

    return;
}



DEFUN_DLD (riSetNNCProperty, args, nargout,
           "Usage:\n"
           "\n"
           "\triSetNNCProperty(Matrix[numNNCConnections][numTimeSteps], [CaseId], PropertyName, [TimeStepIndices]) \n"
           "\n"
           "Interprets the supplied matrix as a property set defined for the NNC connections in the case, "
           "and puts the data into ResInsight as a \"Generated\" property with the name \"PropertyName\"."
           "The \"TimeStepIndices\" argument is used to \"label\" all the time steps present in the supplied data matrix "
           "and must thus be complete. The time step data will then be put into ResInsight at the time steps requested.\n"
           "If the CaseId is not defined, ResInsight's Current Case is used."
           )
{
    int nargin = args.length ();
    if (nargin < 2)
    {
        error("riSetNNCProperty: Too few arguments. The data matrix and the name of the property requested is necessary\n");
        print_usage();
        return octave_value_list ();
    }

    if (nargin > 4)
    {
        error("riSetNNCProperty: Too many arguments.\n");
        print_usage();
        return octave_value_list ();
    }

    Matrix propertyFrames = args(0).matrix_value();


    dim_vector mxDims = propertyFrames.dims();
    if (mxDims.length() != 2)
    {
        error("riSetNNCProperty: The supplied Data Matrix must have two dimensions: numNNCConnections*numTimesteps");
        print_usage();

        return octave_value_list ();
    }
    std::vector<int> argIndices;
    argIndices.push_back(0);
    argIndices.push_back(1);
    argIndices.push_back(2);
    argIndices.push_back(3);

    // Check if we have a CaseId:
    if (!riOctavePlugin::isOctaveValueNumeric(args(argIndices[1])))
    {
        argIndices[1] = -1;
        for (size_t aIdx = 2; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have a Requested TimeSteps
    if (!(nargin > argIndices[3] && args(argIndices[3]).is_matrix_type() && !args(argIndices[3]).is_string()))
    {
        argIndices[3] = -1;
        for (size_t aIdx = 4; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    int caseId = -1;
    std::string propertyName = "UNDEFINED";
    int32NDArray requestedTimeSteps;

    if (argIndices[1] >= 0) caseId              = args(argIndices[1]).int_value();
    if (argIndices[2] >= 0) propertyName        = args(argIndices[2]).char_matrix_value().row_as_string(0);
    if (argIndices[3] >= 0) requestedTimeSteps  = args(argIndices[3]).int32_array_value();

    if (requestedTimeSteps.numel())
    {

        int timeStepCount = mxDims.elem(1);
        if (requestedTimeSteps.numel() != timeStepCount)
        {
            error("riSetNNCProperty: The number of time steps in the input matrix must match the number of time steps in the TimeStepIndices array.");
            print_usage();
            return octave_value_list ();
        }
    }

    setNNCProperty(propertyFrames, "127.0.0.1", riOctavePlugin::portNumber(), caseId, propertyName.c_str(), requestedTimeSteps);

    return octave_value_list ();
}

