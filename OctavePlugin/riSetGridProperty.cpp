#include <QtNetwork>
#include <QStringList>

#include <octave/oct.h>

#include "riSettings.h"
#include "RiaSocketDataTransfer.cpp"  // NB! Include cpp-file to avoid linking of additional file in oct-compile configuration


void setEclipseProperty(const NDArray& propertyFrames, const QString &hostName, quint16 port,
                        const qint64& caseId, const qint64& gridIndex, QString propertyName, const int32NDArray& timeStepIndices, QString porosityModel)
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

    QString command = QString("SetGridProperty %1 %2 %3 %4").arg(caseId).arg(gridIndex).arg(propertyName).arg(porosityModel);

    for (int i = 0; i < timeStepIndices.length(); ++i)
    {
        if (i == 0) command += " ";
        command += QString::number(static_cast<int>(timeStepIndices.elem(i)) - 1); // To make the index 0-based
        if (i != timeStepIndices.length() -1) command += " ";
    }

    QByteArray cmdBytes = command.toLatin1();

    socketStream << (qint64)(cmdBytes.size());
    socket.write(cmdBytes);

    // Write property data header

    dim_vector mxDims = propertyFrames.dims();

    qint64 cellCountI = mxDims.elem(0);
    qint64 cellCountJ = mxDims.elem(1);
    qint64 cellCountK = mxDims.elem(2);
    
    qint64 timeStepCount = 0;
    if (mxDims.length() > 3)
    {
        timeStepCount = mxDims.elem(3);
    }
    else
    {
        timeStepCount = 1;
    }

    qint64 singleTimeStepByteCount = cellCountI * cellCountJ * cellCountK * sizeof(double);

    //octave_stdout << " Cell count I: " << cellCountI << " Cell count J: " << cellCountJ << " Cell count K: " << cellCountK << std::endl;
    //octave_stdout << " Time step count: " << timeStepCount <<  std::endl;

    socketStream << (qint64)(cellCountI);
    socketStream << (qint64)(cellCountJ);
    socketStream << (qint64)(cellCountK);
    socketStream << (qint64)(timeStepCount);
    socketStream << (qint64)singleTimeStepByteCount;

    const double* internalData = propertyFrames.fortran_vec();

    QStringList errorMessages;
    if (!RiaSocketDataTransfer::writeBlockDataToSocket(&socket, (const char *)internalData, timeStepCount*singleTimeStepByteCount, errorMessages))
    {
        for (int i = 0; i < errorMessages.size(); i++)
        {
            octave_stdout << errorMessages[i].toStdString();
        }

        size_t cellCount = cellCountI * cellCountJ * cellCountK;
        error("riSetGridProperty : Was not able to write the proper amount of data to ResInsight:");
        octave_stdout << " Cell count : " << cellCount << "Time steps : " << timeStepCount << std::endl;

        return;
    }
    
    QString tmp = QString("riSetGridProperty : Wrote %1").arg(propertyName);

    if (caseId == -1)
    {
        tmp += QString(" to current case,");
    }
    else
    {
        tmp += QString(" to case with Id = %1,").arg(caseId);
    }
        
    tmp += QString(" grid index: %1, ").arg(gridIndex);

    octave_stdout << tmp.toStdString() << " Time steps : " << timeStepCount << std::endl;

    while(socket.bytesToWrite() && socket.state() == QAbstractSocket::ConnectedState)
    {
//        octave_stdout << "Bytes to write: " << socket.bytesToWrite() << std::endl << std::flush;
        socket.waitForBytesWritten(riOctavePlugin::shortTimeOutMilliSecs);
        OCTAVE_QUIT;
    }

    if (socket.bytesToWrite() && socket.state() != QAbstractSocket::ConnectedState)
    {
        error("riSetGridProperty : ResInsight refused to accept the data. Maybe the dimensions or porosity model is wrong.\n");
    }

#ifdef WIN32
    // TODO: Due to synchronization issues seen on Windows 10, it is required to do a sleep here to be able to catch disconnect
    // signals from the socket. No sleep causes the server to hang.
    Sleep(100);
#endif //WIN32

    return;
}



DEFUN_DLD (riSetGridProperty, args, nargout,
           "Usage:\n"
           "\n"
           "\triSetGridProperty( Matrix[numI][numJ][numK][numTimeSteps], [CaseId], GridIndex, PropertyName, [TimeStepIndices], [PorosityModel = \"Matrix\"|\"Fracture\"] ) \n"
           "\n"
           "Interprets the supplied matrix as a property set defined for all cells in one of the grids in a case, and puts the data into ResInsight as a \"Generated\" property with the name \"PropertyName\".\n"
           "The \"TimeStepIndices\" argument is used to \"label\" all the time steps present in the supplied data matrix, and must thus be complete. The time step data will then be put into ResInsight at the time steps requested."
           "If the CaseId is not defined, ResInsight’s Current Case is used.\n"
           )
{
    int nargin = args.length ();
    if (nargin < 2)
    {
        error("riSetGridProperty: Too few arguments, required input parameters are the data matrix, grid index and property name.\n");
        print_usage();
        return octave_value_list ();
    }

    if (nargin > 6)
    {
        error("riSetGridProperty: Too many arguments.\n");
        print_usage();
        return octave_value_list ();
    }

    NDArray propertyFrames = args(0).array_value();

    if (error_state)
    {
        error("riSetGridProperty: The supplied first argument is not a valid Matrix");
        print_usage();

        return octave_value_list ();
    }


    dim_vector mxDims = propertyFrames.dims();
    if (!(mxDims.length() == 3 || mxDims.length() == 4))
    {
        error("riSetGridProperty: The supplied Data Matrix must have three dimensions (numI*numJ*numK*1) or four dimensions (numI*numJ*numK*numTimeSteps)");
        print_usage();

        return octave_value_list ();
    }
    std::vector<int> argIndices;
    argIndices.push_back(0); // Array data
    argIndices.push_back(1); // Case Id
    argIndices.push_back(2); // GridIndex
    argIndices.push_back(3); // Property name
    argIndices.push_back(4); // Time step indices
    argIndices.push_back(5); // Porosity model

    // Check that the second argument is an integer
    if (!args(argIndices[1]).is_real_scalar()) // Check if second argument is an int. If it is
    {
        error("riSetGridProperty: The GridIndex argument is missing");
        print_usage();

        return octave_value_list ();
    }

    // Check if we do not have a CaseId:
    if (args(argIndices[2]).is_string()) // Check if second argument is a text. If it is, the caseid is missing
    {
        argIndices[1] = -1;
        for (size_t aIdx = 2; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have a Requested TimeSteps

    if (!(nargin > argIndices[4] && args(argIndices[4]).is_matrix_type() && !args(argIndices[4]).is_string()))
    {
        argIndices[4] = -1;
        for (size_t aIdx = 5; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have a PorosityModel

    int lastArgumentIndex = argIndices[5] ;
    if (!(nargin > argIndices[5] && args(argIndices[5]).is_string()))
    {
        argIndices[5] = -1;
        for (size_t aIdx = 6; aIdx < argIndices.size(); ++aIdx)
            --argIndices[aIdx];
    }

    // Check if we have more arguments than we should
    if (nargin > lastArgumentIndex + 1)
    {
        error("riSetGridProperty: Unexpected argument after the PorosityModel.\n");
        print_usage();
        return octave_value_list ();
    }


    int caseId = -1;
    int gridIndex = 0;
    std::string propertyName = "UNDEFINED";
    int32NDArray timeStepIndices;
    std::string porosityModel = "Matrix";

    if (argIndices[1] >= 0) caseId          = args(argIndices[1]).int_value();
    if (argIndices[2] >= 0) gridIndex       = args(argIndices[2]).int_value();
    if (argIndices[3] >= 0) propertyName    = args(argIndices[3]).char_matrix_value().row_as_string(0);
    if (argIndices[4] >= 0) timeStepIndices = args(argIndices[4]).int32_array_value();
    if (argIndices[5] >= 0) porosityModel   = args(argIndices[5]).string_value();

    if (timeStepIndices.length() > 1)
    {
        if (mxDims.length() == 3)
        {
            error("riSetGridProperty: The input matrix has three dimensions, but there are more than one time step in [TimeStepIndices]. If more than one time step is defined, the data matrix must be 4D.");
            print_usage();
            return octave_value_list ();
        }
        
        int timeStepCount = mxDims.elem(3);
        if (timeStepIndices.length() != timeStepCount)
        {
            error("riSetGridProperty: The number of time steps in the input matrix must match the number of time steps in the TimeStepIndices array.");
            print_usage();
            return octave_value_list ();
        }
    }

    if (porosityModel != "Matrix" && porosityModel != "Fracture")
    {
        error("riSetGridProperty: The value for \"PorosityModel\" is unknown. Please use either \"Matrix\" or \"Fracture\"\n");
        print_usage();
        return octave_value_list ();
    }

    setEclipseProperty(propertyFrames, "127.0.0.1", 40001, caseId, gridIndex, propertyName.c_str(), timeStepIndices, porosityModel.c_str());

    return octave_value_list ();
}

