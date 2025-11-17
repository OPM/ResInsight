/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimOpmFlowJobSettings.h"

#include "cafPdmFieldCapability.h"
#include "cafPdmUiCheckBoxAndTextEditor.h"
#include "cafPdmUiCheckBoxEditor.h"
#include "cafPdmUiListEditor.h"

#include <QFile>

CAF_PDM_SOURCE_INIT( RimOpmFlowJobSettings, "OpmFlowJobSettings" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOpmFlowJobSettings::RimOpmFlowJobSettings()
{
    CAF_PDM_InitObject( "Opm Flow Job Settings" );

    CAF_PDM_InitField( &m_mpiProcesses, "mpiProcesses", 2, "Number of MPI Processes" );
    CAF_PDM_InitField( &m_threadsPerProcess,
                       "threadsPerProcess",
                       2,
                       "Threads Per Process",
                       "",
                       "The maximum number of threads to be instantiated per process ('-1' means 'automatic')." );

    CAF_PDM_InitField( &m_enableEsmry, "enableEsmry", true, "Enable ESMRY Output", "", "Write ESMRY file for fast loading of summary data." );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_enableEsmry );
    CAF_PDM_InitField( &m_enableTerminalOutput,
                       "enableTerminalOutput",
                       true,
                       "Enable Terminal Output",
                       "",
                       "Print high-level information about the simulation's progress to the terminal." );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_enableTerminalOutput );
    CAF_PDM_InitField( &m_enableTuning, "enableTuning", false, "Enable Tuning", "", "Honor some aspects of the TUNING keyword." );
    caf::PdmUiNativeCheckBoxEditor::configureFieldForEditor( &m_enableTuning );

    CAF_PDM_InitField( &m_ignoreKeywords,
                       "ignoreKeywords",
                       std::vector<QString>{},
                       "Ignore Keywords",
                       "",
                       "List of Eclipse keywords which should be ignored." );
    m_ignoreKeywords.uiCapability()->setUiEditorTypeName( caf::PdmUiListEditor::uiEditorTypeName() );

    CAF_PDM_InitField( &m_newtonMaxIterations,
                       "newtonMaxIterations",
                       std::make_pair( false, 20 ),
                       "Newton Max Iterations",
                       "",
                       "The maximum number of Newton iterations per time step." );
    m_newtonMaxIterations.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_parsingStrictness,
                       "parsingStrictness",
                       QString( "normal" ),
                       "Parsing Strictness",
                       "",
                       "Set strictness of parsing process. Normal = stop for critical errors, High = stop for all "
                       "errors,  Low = Do not stop due to unsupported keywords even if marked critical." );
    CAF_PDM_InitField( &m_relaxedMaxPvFraction,
                       "relaxedMaxPvFraction",
                       std::make_pair( false, 0.03 ),
                       "Relaxed Max PV Fraction",
                       "",
                       "The fraction of the pore volume of the reservoir where the volumetric error (CNV) may be "
                       "violated during strict Newton iterations." );
    CAF_PDM_InitField( &m_solverMaxTimeStepInDays,
                       "solverMaxTimeStepInDays",
                       std::make_pair( false, 365.0 ),
                       "Solver Max Time Step In Days",
                       "",
                       "The maximum size of a time step in days." );
    CAF_PDM_InitField( &m_solverMinTimeStepInDays,
                       "solverMinTimeStepInDays",
                       std::make_pair( false, 1e-12 ),
                       "Solver Min Time Step In Days",
                       "",
                       "The minimum size of a time step in days for field and metric and hours for lab. If a step "
                       "cannot converge without getting cut below this step size the simulator will stop." );
    CAF_PDM_InitField( &m_minStrictCnvIter,
                       "minStrictCnvIter",
                       std::make_pair( false, -1 ),
                       "Min Strict CNV Iter",
                       "",
                       "Minimum number of Newton iterations before relaxed tolerances can be used for the CNV "
                       "convergence criterion." );
    m_minStrictCnvIter.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_minStrictMbIter,
                       "minStrictMbIter",
                       std::make_pair( false, -1 ),
                       "Min Strict MB Iter",
                       "",
                       "Minimum number of Newton iterations before relaxed tolerances can be used for the MB "
                       "convergence criterion." );
    m_minStrictMbIter.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );
    CAF_PDM_InitField( &m_minTimeStepBasedOnNewtonIterations,
                       "minTimeStepBasedOnNewtonIterations",
                       std::make_pair( false, 0.0 ),
                       "Min Time Step Based On Newton Iterations",
                       "",
                       "The minimum time step size (in days for field and metric unit and hours for lab unit) "
                       "can be reduced to based on newton iteration counts." );
    CAF_PDM_InitField( &m_minTimeStepBeforeShuttingProblematicWellsInDays,
                       "minTimeStepBeforeShuttingProblematicWellsInDays",
                       std::make_pair( false, 0.01 ),
                       "Min Time Step Before Shutting Problematic Wells In Days",
                       "",
                       "The minimum time step size in days for which problematic wells are not shut." );
    CAF_PDM_InitField( &m_toleranceCnv,
                       "toleranceCnv",
                       std::make_pair( false, 0.01 ),
                       "Tolerance CNV",
                       "",
                       "Local convergence tolerance (Maximum of local saturation errors)." );
    CAF_PDM_InitField( &m_toleranceCnvEnergy,
                       "toleranceCnvEnergy",
                       std::make_pair( false, 0.01 ),
                       "Tolerance CNV Energy",
                       "",
                       "Local energy convergence tolerance (Maximum of local energy errors)." );
    CAF_PDM_InitField( &m_toleranceCnvEnergyRelaxed,
                       "toleranceCnvEnergyRelaxed",
                       std::make_pair( false, 1.0 ),
                       "Tolerance CNV Energy Relaxed",
                       "",
                       "Relaxed local energy convergence tolerance that applies for iterations after the iterations with the strict "
                       "tolerance." );
    CAF_PDM_InitField( &m_toleranceCnvRelaxed,
                       "toleranceCnvRelaxed",
                       std::make_pair( false, 1.0 ),
                       "Tolerance CNV Relaxed",
                       "",
                       "Relaxed local convergence tolerance that applies for iterations after the iterations with the strict tolerance." );
    CAF_PDM_InitField( &m_toleranceEnergyBalance,
                       "toleranceEnergyBalance",
                       std::make_pair( false, 1e-07 ),
                       "Tolerance Energy Balance",
                       "",
                       "Energy balance convergence tolerance (Maximum of global energy balance error)." );
    CAF_PDM_InitField( &m_toleranceEnergyBalanceRelaxed,
                       "toleranceEnergyBalanceRelaxed",
                       std::make_pair( false, 1e-06 ),
                       "Tolerance Energy Balance Relaxed",
                       "",
                       "Relaxed energy balance convergence tolerance that applies for iterations after the iterations with the strict "
                       "tolerance." );
    CAF_PDM_InitField( &m_toleranceMb,
                       "toleranceMb",
                       std::make_pair( false, 1e-07 ),
                       "Tolerance MB",
                       "",
                       "Global mass balance convergence tolerance (Maximum of global mass balance errors)." );
    CAF_PDM_InitField( &m_toleranceMbRelaxed,
                       "toleranceMbRelaxed",
                       std::make_pair( false, 1e-06 ),
                       "Tolerance MB Relaxed",
                       "",
                       "Relaxed global mass balance convergence tolerance that applies for iterations after the iterations with the strict "
                       "tolerance." );
    CAF_PDM_InitField( &m_tolerancePressureMsWells,
                       "tolerancePressureMsWells",
                       std::make_pair( false, 1000.0 ),
                       "Tolerance Pressure MS Wells",
                       "",
                       "Convergence tolerance for pressure in multi-segment wells." );
    CAF_PDM_InitField( &m_toleranceWellControl,
                       "toleranceWellControl",
                       std::make_pair( false, 1e-07 ),
                       "Tolerance Well Control",
                       "",
                       "Convergence tolerance for well control equations." );
    CAF_PDM_InitField( &m_toleranceWells, "toleranceWells", std::make_pair( false, 0.0001 ), "Tolerance Wells", "", "Well convergence tolerance." );
    CAF_PDM_InitField( &m_wellGroupConstraintsMaxIterations,
                       "wellGroupConstraintsMaxIterations",
                       std::make_pair( false, 1 ),
                       "Well Group Constraints Max Iterations",
                       "",
                       "Maximum number of iterations for well group constraints." );
    m_wellGroupConstraintsMaxIterations.uiCapability()->setUiEditorTypeName( caf::PdmUiCheckBoxAndTextEditor::uiEditorTypeName() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOpmFlowJobSettings::~RimOpmFlowJobSettings()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimOpmFlowJobSettings* RimOpmFlowJobSettings::clone()
{
    return copyObject<RimOpmFlowJobSettings>();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJobSettings::uiOrdering( caf::PdmUiGroup* uiGroup, bool expandByDefault )
{
    caf::PdmUiGroup* generalGroup = uiGroup->addNewGroup( "Process Control" );
    if ( !expandByDefault ) generalGroup->setCollapsedByDefault();
    generalGroup->add( &m_mpiProcesses );
    generalGroup->add( &m_threadsPerProcess );

    caf::PdmUiGroup* optionsGroup = uiGroup->addNewGroup( "Simulator Options" );
    if ( !expandByDefault ) optionsGroup->setCollapsedByDefault();
    optionsGroup->add( &m_enableEsmry );
    optionsGroup->add( &m_enableTerminalOutput );
    optionsGroup->add( &m_enableTuning );
    optionsGroup->add( &m_ignoreKeywords );
    optionsGroup->add( &m_parsingStrictness );

    caf::PdmUiGroup* solverGroup = uiGroup->addNewGroup( "Solver Settings" );
    solverGroup->setCollapsedByDefault();
    solverGroup->add( &m_newtonMaxIterations );
    solverGroup->add( &m_relaxedMaxPvFraction );
    solverGroup->add( &m_solverMaxTimeStepInDays );
    solverGroup->add( &m_solverMinTimeStepInDays );
    solverGroup->add( &m_minStrictCnvIter );
    solverGroup->add( &m_minStrictMbIter );
    solverGroup->add( &m_minTimeStepBasedOnNewtonIterations );
    solverGroup->add( &m_minTimeStepBeforeShuttingProblematicWellsInDays );
    solverGroup->add( &m_wellGroupConstraintsMaxIterations );

    auto tolerancesGroup = uiGroup->addNewGroup( "Convergence Tolerances" );
    tolerancesGroup->setCollapsedByDefault();
    tolerancesGroup->add( &m_toleranceCnv );
    tolerancesGroup->add( &m_toleranceCnvRelaxed );
    tolerancesGroup->add( &m_toleranceCnvEnergy );
    tolerancesGroup->add( &m_toleranceCnvEnergyRelaxed );
    tolerancesGroup->add( &m_toleranceMb );
    tolerancesGroup->add( &m_toleranceMbRelaxed );
    tolerancesGroup->add( &m_toleranceEnergyBalance );
    tolerancesGroup->add( &m_toleranceEnergyBalanceRelaxed );
    tolerancesGroup->add( &m_tolerancePressureMsWells );
    tolerancesGroup->add( &m_toleranceWellControl );
    tolerancesGroup->add( &m_toleranceWells );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimOpmFlowJobSettings::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_parsingStrictness )
    {
        for ( QString name : { "Low", "Normal", "High" } )
        {
            options.push_back( caf::PdmOptionItemInfo( name, QVariant::fromValue( name.toLower() ) ) );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RimOpmFlowJobSettings::mpiProcesses() const
{
    return m_mpiProcesses;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QStringList RimOpmFlowJobSettings::commandLineOptions( QString resInsightWorkDir, QString flowWorkDir ) const
{
    QStringList options;

    options << QString( "--threads-per-process=%1" ).arg( m_threadsPerProcess );

    QString trueOrFalse = m_enableEsmry ? "true" : "false";
    options << "--enable-esmry=" + trueOrFalse;

    trueOrFalse = m_enableTerminalOutput ? "true" : "false";
    options << "--enable-terminal-output=" + trueOrFalse;

    trueOrFalse = m_enableTuning ? "true" : "false";
    options << "--enable-tuning=" + trueOrFalse;

    if ( !m_ignoreKeywords().empty() )
    {
        QStringList kws;
        for ( const QString& kw : m_ignoreKeywords() )
        {
            kws << kw;
        }
        options << QString( "--ignore-keywords=%1" ).arg( kws.join( ":" ) );
    }

    options << QString( "--parsing-strictness=%1" ).arg( m_parsingStrictness() );

    if ( m_newtonMaxIterations().first )
    {
        options << QString( "--newton-max-iterations=%1" ).arg( m_newtonMaxIterations().second );
    }
    if ( m_relaxedMaxPvFraction().first )
    {
        options << QString( "--relaxed-max-pv-fraction=%1" ).arg( m_relaxedMaxPvFraction().second );
    }
    if ( m_solverMaxTimeStepInDays().first )
    {
        options << QString( "--solver-max-time-step-in-days=%1" ).arg( m_solverMaxTimeStepInDays().second );
    }
    if ( m_solverMinTimeStepInDays().first )
    {
        options << QString( "--solver-min-time-step=%1" ).arg( m_solverMinTimeStepInDays().second );
    }
    if ( m_minStrictCnvIter().first )
    {
        options << QString( "--min-strict-cnv-iter=%1" ).arg( m_minStrictCnvIter().second );
    }
    if ( m_minStrictMbIter().first )
    {
        options << QString( "--min-strict-mb-iter=%1" ).arg( m_minStrictMbIter().second );
    }
    if ( m_minTimeStepBasedOnNewtonIterations().first )
    {
        options << QString( "--min-time-step-based-on-newton-iterations=%1" ).arg( m_minTimeStepBasedOnNewtonIterations().second );
    }
    if ( m_minTimeStepBeforeShuttingProblematicWellsInDays().first )
    {
        options << QString( "--min-time-step-before-shutting-problematic-wells-in-days=%1" )
                       .arg( m_minTimeStepBeforeShuttingProblematicWellsInDays().second );
    }
    if ( m_toleranceCnv().first )
    {
        options << QString( "--tolerance-cnv=%1" ).arg( m_toleranceCnv().second );
    }
    if ( m_toleranceCnvRelaxed().first )
    {
        options << QString( "--tolerance-cnv-relaxed=%1" ).arg( m_toleranceCnvRelaxed().second );
    }
    if ( m_toleranceCnvEnergy().first )
    {
        options << QString( "--tolerance-cnv-energy=%1" ).arg( m_toleranceCnvEnergy().second );
    }
    if ( m_toleranceCnvEnergyRelaxed().first )
    {
        options << QString( "--tolerance-cnv-energy-relaxed=%1" ).arg( m_toleranceCnvEnergyRelaxed().second );
    }
    if ( m_toleranceMb().first )
    {
        options << QString( "--tolerance-mb=%1" ).arg( m_toleranceMb().second );
    }
    if ( m_toleranceMbRelaxed().first )
    {
        options << QString( "--tolerance-mb-relaxed=%1" ).arg( m_toleranceMbRelaxed().second );
    }
    if ( m_toleranceEnergyBalance().first )
    {
        options << QString( "--tolerance-energy-balance=%1" ).arg( m_toleranceEnergyBalance().second );
    }
    if ( m_toleranceEnergyBalanceRelaxed().first )
    {
        options << QString( "--tolerance-energy-balance-relaxed=%1" ).arg( m_toleranceEnergyBalanceRelaxed().second );
    }
    if ( m_tolerancePressureMsWells().first )
    {
        options << QString( "--tolerance-pressure-ms-wells=%1" ).arg( m_tolerancePressureMsWells().second );
    }
    if ( m_toleranceWellControl().first )
    {
        options << QString( "--tolerance-well-control=%1" ).arg( m_toleranceWellControl().second );
    }
    if ( m_toleranceWells().first )
    {
        options << QString( "--tolerance-wells=%1" ).arg( m_toleranceWells().second );
    }
    if ( m_wellGroupConstraintsMaxIterations().first )
    {
        options << QString( "--well-group-constraints-max-iterations=%1" ).arg( m_wellGroupConstraintsMaxIterations().second );
    }

    if ( options.length() > 2 )
    {
        QString paramName( "/opmflow.params" );
        QFile   workDirFile( resInsightWorkDir + paramName );

        if ( workDirFile.open( QFile::WriteOnly | QFile::Text ) )
        {
            for ( const QString& option : options )
            {
                QByteArray line = option.mid( 2 ).toUtf8() + "\n";
                workDirFile.write( line );
            }
            workDirFile.close();
            options.clear();
            options << QString( "--parameter-file=%1" ).arg( flowWorkDir + paramName );
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimOpmFlowJobSettings::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    if ( field == &m_ignoreKeywords )
    {
        auto attrib = dynamic_cast<caf::PdmUiListEditorAttribute*>( attribute );
        if ( attrib )
        {
            attrib->heightHint = 40;
        }
    }
}