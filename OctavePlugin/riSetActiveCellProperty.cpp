#include <QtNetwork>
#include <QStringList>

#include <octave/oct.h>

#include "riSettings.h"
#include "RiaSocketDataTransfer.cpp"  // NB! Include cpp-file to avoid linking of additional file in oct-compile configuration


void setEclipseProperty(const Matrix& propertyFrames, const QString &hostName, quint16 port,
                        const qint64& caseId, QString propertyName, const int32NDArray& requestedTimeSteps, QString porosityModel)
{
    QTcpSocket socket;
    socket.connectToHost(hostName, port);

    if (!socket.waitForConnected(riOctavePlugin::connectTimeOutMilliSecs))
    {
        error((("Connection: ") + socket.errorString()).toLatin1().data());
        return;
    }

    QDataStream socketStream(&socket);
    socketStream.setVersion(riOctavePlugin::qtDataStreamVersion);

    // Create command as a string with arguments , and send it:

    QString command;
    command += "SetActiveCellProperty " + QString::number(caseId) + " " + propertyName + " " + porosityModel;

    for (int i = 0; i < requestedTimeSteps.length(); ++i)
    {
        if (i == 0) command += " ";
        command += QString::number(static_cast<int>(requestedTimeSteps.elem(i)) - 1); // To make the index 0-based
        if (i != requestedTimeSteps.length() -1) command += " ";
    }

    QByteArray cmdBytes = command.toLatin1();

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Write property data header

    dim_vector mxDims = propertyFrames.dims();

    qint64 cellCount = mxDims.elem(0);
    qint64 timeStepCount = mxDims.elem(1);
    qint64 timeStepByteCount = cellCount * sizeof(double);

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

    QString tmp = QString("riSetActiveCellProperty : Wrote %1").arg(propertyName);

    if (caseId == -1)
    {
        tmp += QString(" to current case.");
    }
    else
    {
        tmp += QString(" to case with Id = %1.").arg(caseId);
    }
    octave_stdout << tmp.toStdString() << " Active Cells : " << cellCount << " Time steps : " << timeStepCount << std::endl;

    while(socket.bytesToWrite() && socket.state() == QAbstractSocket::ConnectedState)
    {
        // octave_stdout << "Bytes to write: " << socket.bytesToWrite() << std::endl;
        socket.waitForBytesWritten(riOctavePlugin::shortTimeOutMilliSecs);
        OCTAVE_QUIT;
    }

    //octave_stdout << "    Socket write completed" << std::endl;

    if (socket.bytesToWrite() && socket.state() != QAbstractSocket::ConnectedState)
    {
        error("riSetActiveCellProperty : ResInsight refused to accept the data. Maybe the dimensions or porosity model is wrong");
    }

#ifdef WIN32
    // TODO: Due to synchronization issues seen on Windows 10, it is required to do a sleep here to be able to catch disconnect
    // signals from the socket. No sleep causes the server to hang.
    Sleep(100);
#endif //WIN32

    return;
}



DEFUN_DLD (riSetActiveCellProperty, args, nargout,
           "Usage:\n"
           "\n"
           "\triSetActiveCellProperty( Matrix[numActiveCells][numTimeSteps], [CaseId], PropertyName, [TimeStepIndices], [PorosityModel = \"Matrix\"|\"Fracture\"] ) \n"
           "\n"
           "Interprets the supplied matrix as a property set defined for the active cells in the case, "
           "and puts the data into ResInsight as a \"Generated\" property with the name \"PropertyName\"."
           "The \"TimeStepIndices\" argument is used to \"label\" all the time steps present in the supplied data matrix,"
           "and must thus be complete. The time step data will then be put into ResInsight at the time steps requested."  
           "If the CaseId is not defined, ResInsight’s Current Case is used."
           )
{
    int nargin = args.length ();
    if (nargin < 2)
    {
        error("riSetActiveCellProperty: Too few arguments. The data matrix and the name of the property requested is neccesary\n");
        print_usage();
        return octave_value_list ();
    }

    if (nargin > 5)
    {
        error("riSetActiveCellProperty: Too many arguments.\n");
        print_usage();
        return octave_value_list ();
    }

    Matrix propertyFrames = args(0).matrix_value();

    if (error_state)
    {
        error("riSetActiveCellProperty: The supplied first argument is not a valid Matrix");
        print_usage();

        return octave_value_list ();
    }


    dim_vector mxDims = propertyFrames.dims();
    if (mxDims.length() != 2)
    {
        error("riSetActiveCellProperty: The supplied Data Matrix must have two dimensions: NumActiveCells*numTimesteps");
        print_usage();

        return octave_value_list ();
    }
    std::vector<int> argIndices;
    argIndices.push_back(0);
    argIndices.push_back(1);
    argIndices.push_back(2);
    argIndices.push_back(3);
    argIndices.push_back(4);

    // Check if we have a CaseId:
    if (!args(argIndices[1]).is_numeric_type())
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

    // Check if we have a PorosityModel

    int lastArgumentIndex = argIndices[4] ;
    if (!(nargin > argIndices[4] && args(argIndices[4]).is_string()))
    {
        argIndices[4] = -1;
        for (size_t aIdx = 5; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have more arguments than we should
    if (nargin > lastArgumentIndex + 1)
    {
        error("riSetActiveCellProperty: Unexpected argument after the PorosityModel.\n");
        print_usage();
        return octave_value_list ();
    }


    int caseId = -1;
    std::string propertyName = "UNDEFINED";
    int32NDArray requestedTimeSteps;
    std::string porosityModel = "Matrix";

    if (argIndices[1] >= 0) caseId              = args(argIndices[1]).int_value();
    if (argIndices[2] >= 0) propertyName        = args(argIndices[2]).char_matrix_value().row_as_string(0);
    if (argIndices[3] >= 0) requestedTimeSteps  = args(argIndices[3]).int32_array_value();
    if (argIndices[4] >= 0) porosityModel       = args(argIndices[4]).string_value();

    if (requestedTimeSteps.length())
    {

        int timeStepCount = mxDims.elem(1);
        if (requestedTimeSteps.length() != timeStepCount)
        {
            error("riSetActiveCellProperty: The number of timesteps in the input matrix must match the number of timesteps in the TimeStepIndices array.");
            print_usage();
            return octave_value_list ();
        }
    }

    if (porosityModel != "Matrix" && porosityModel != "Fracture")
    {
        error("riSetActiveCellProperty: The value for \"PorosityModel\" is unknown. Please use either \"Matrix\" or \"Fracture\"\n");
        print_usage();
        return octave_value_list ();
    }

    setEclipseProperty(propertyFrames, "127.0.0.1", 40001, caseId, propertyName.c_str(), requestedTimeSteps, porosityModel.c_str());

    return octave_value_list ();
}

