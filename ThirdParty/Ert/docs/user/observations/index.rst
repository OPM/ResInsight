Configuring observations for ERT
================================

General overview
----------------

When using ERT to condition on dynamic data, it is necessary to
specify which data to condition on. In particular, for a given piece
of data to condition on, the ERT  application needs to know:

 - The actual measured value of the data.
 - The uncertainty of the measured data.
 - The time of measurement.
 - How to simulate a response of the data given a parametrized ECLIPSE model. 


To provide this observation to ERT, an observation file must be
created. The observation file is a plain text file, and is in essence
built around for different classes of observations and has an
associated keyword for each class:

 - Well or group rates from an existing ECLIPSE reference case: The
   HISTORY_OBSERVATION keyword.

 - Well logs, RFTS and PLTs: The BLOCK_OBSERVATION keyword. 

 - Separator tests, region pressures, etc.: The SUMMARY_OBSERVATION
   keyword.

 - Exotic observations (e.g. data from 4D seismic): The
   GENERAL_OBSERVATION keyword.


The HISTORY_OBSERVATION keyword
-------------------------------

The keyword HISTORY_OBSERVATION is used to condition on observations
from the WCONHIST and WCONINJH keywords in schedule file provided to
the enkf project (or alternatively an ECLIPSE summary file if you have
changed the HISTORY_SOURCE keyword in the enkf project). The keyword
is typically used to condition on production and injection rates for
groups and wells, as well as bottom hole and tubing head pressures. An
observation entered with the HISTORY_OBSERVATION keyword will be
active at all report steps where data for the observation can be
found.

In it's simplest form, a history observation is created as follows::

 HISTORY_OBSERVATION WOPR:P1;

This will condition on WOPR in well P1 using a default observation
error. The default observation error is a relative error of 10% to the
measurement with a minimum error of 0.10. See below on how explicitly
set the error.

In general, to condition on variable VAR in well or group WGNAME, one
uses::
    
 HISTORY_OBSERVATION VAR:WGNAME;

Note that there must be a colon ":" between VAR and WGNAME and that
the statement shall end with a semi-colon ";". Thus, to condition on
WOPR, WWCT and WGOR in well C-17, and for the GOPR for the whole
field, one would add the following to the observation configuration::
   
 HISTORY_OBSERVATION WOPR:C-17;
 HISTORY_OBSERVATION WWCT:C-17;
 HISTORY_OBSERVATION WGOR:C-17;
 
 HISTORY_OBSERVATION GOPR:FIELD;

By default, the observation error is set to 10% of the observed value,
with a minimum of 0.10. It can be changed as follows::
   
 HISTORY_OBSERVATION GOPR:FIELD
 {
    ERROR       = 1000;
    ERROR_MODE  = ABS;
 };

This will set the observation error to 1000 for all observations of
GOPR:FIELD. Note that both the items ERROR and ERROR_MODE as well as
the whole definition shall end with a semi-colon.

The item ERROR_MODE can take three different values: ABS, REL or
RELMIN. If set to REL, all observation errors will be set to the
observed values multiplied by ERROR. Thus, the following will
condition on water injection rate for the whole field with 20%
observation uncertainity::

 HISTORY_OBSERVATION GWIR:FIELD
 {
    ERROR       = 0.20;
    ERROR_MODE  = REL;
 };

If you do not want the observation error to drop below a given
threshold, say 100, you can use RELMIN and the keyword ERROR_MIN::

 HISTORY_OBSERVATION GWIR:FIELD
 {
    ERROR       = 0.20;
    ERROR_MODE  = RELMIN;
    ERROR_MIN   = 100;
 };

Note that the configuration parser does not threat carriage return
different from space. Thus, the following statement is equivalent to
the previous::

 HISTORY_OBSERVATION GWIR:FIELD { ERROR = 0.20; ERROR_MODE = RELMIN; ERROR_MIN = 100; };


Also note that the special keyword include can be used to read an
external file. This can be very useful if you want to change the
standard configuration for a lot of observations in one go. For
example, consider the following code::

 HISTORY_OBSERVATION WOPR:P1 { include "hist_obs_wells.txt"; };
 HISTORY_OBSERVATION WOPR:P2 { include "hist_obs_wells.txt"; };
 HISTORY_OBSERVATION WOPR:P3 { include "hist_obs_wells.txt"; };
 HISTORY_OBSERVATION WOPR:P4 { include "hist_obs_wells.txt"; };
 HISTORY_OBSERVATION WOPR:P5 { include "hist_obs_wells.txt"; };

Where the contents of the file hist_obs_wells.txt may be something
like::

 ERROR_MODE  = RELMIN;
 ERROR       = 0.25;
 ERROR_MIN   = 100;

In this case, changing the file hist_obs_wells.txt will affect all of
the observations.

Note that the keyword include can be used anywhere in the
configuration file. However, nested inclusion (use of include in a
file that has already been included with include) is not allowed.

By default, an observation entered with the HISTORY_OBSERVATION
keyword will get the observed values, i.e. the 'true' values, from the
WCONHIST and WCONINJH keywords in the schedule file provided to the
ERT project. However it also possible to get the observed values from
a reference case. In that case you must set set HISTORY_SOURCE
variable in the ERT configuration file, see Creating a configuration
file for ERT.

To change the observation error for a HISTORY_OBSERVATION for one or
more segments of the historic period, you can use the SEGMENT
keyword. For example::

  HISTORY_OBSERVATION GWIR:FIELD
  {
     ERROR       = 0.20;
     ERROR_MODE  = RELMIN;
     ERROR_MIN   = 100;

     SEGMENT FIRST_YEAR
     {
        START = 0;
        STOP  = 10;
        ERROR = 0.50;
        ERROR_MODE = REL;
     };

     SEGMENT SECOND_YEAR
     {
        START      = 11;
        STOP       = 20;
        ERRROR     = 1000;
        ERROR_MODE = ABS;
     };
  };

The items START and STOP sets the start and stop of the segment in
terms of ECLIPSE restart steps. The keywords ERROR, ERROR_MODE and
ERROR_MIN behaves like before. If the segments overlap, they are
computed in alphabetical order.  Error covariance for "merged" updates

When merging the historical observations from several report steps
together in one update the different steps are not independent, and it
is beneficial to use a error covariance matrix, by using the keywords
AUTO_CORRF and AUTO_CORRF_PARAM ERT will automatically estimate a
error-covariance matrix based on the auto correlation function
specified by the AUTO_CORRF keyword, with the parameter given by the
AUTO_CORRF_PARAM parameter (i.e. the auto correlation length). The
currently available auto correlation functions are:

 EXP   ~ exp(-x)
 GAUSS ~ exp(-x*x/2)

where the parameter x is given as:

  x = (t2 - t1) / AUTO_CORRF_PARAM


The SUMMARY_OBSERVATION keyword
-------------------------------

The keyword SUMMARY_OBSERVATION can be used to condition on any
observation whos simulated value is written to the ECLIPSE summary
file, e.g. well rates, region properties, group and field rates etc. A
quite typical usage of SUMMARY_OBSERVATION is to condition on the
results of a separator test.

Note: Although it is possible to condition on well and group rates
with SUMMARY_OBSERVATION, it is usually easier to use
HISTORY_OBSERVATION for this.

In order to create a summary observation, four pieces of information
are needed: The observed value, the observation error, the time of
observation and a summary key. A typical summary observation is
created as follows::

 SUMMARY_OBSERVATION SEP_TEST_2005
 {
    VALUE = 100;
    ERROR =   5;
    DATE  = 21/08/2005;
    KEY   = GOPR:BRENT;
 };

This will create an observation of group oil production for the brent
group on 21th of august 2005. The observed value was 100 with a
standard deviation of 5. The name SEP_TEST_2005 will be used as a
label for the observation within the ERT and must be unique.

Similarly to the name of a HISTORY_OBSERVATION, the item KEY in a
SUMMARY_OBSERVATION is used to look up the simulated value from the
summary file. And again, as when declaring a HISTORY_OBSERVATION, to
condition on VAR in well, group or region WGRNAME, one uses::

 KEY = VAR:WGRNAME;

For example, to condition on RPPW in region 8, one uses::

 KEY = RPPW:8;

It is also possible to give the observation time as a restart number
using the RESTART item or as time in days from simulation start using
the DAYS item. Here are two examples::

 -- Giving the observation time in terms of restart number.
 SUMMARY_OBSERVATION SEP_TEST_2005
 {
    VALUE    = 100;
    ERROR    =   5;
    RESTART  =  42;
    KEY      = GOPR:BRENT;
 };
 

 -- Giving the observation time in terms of days
 -- from simulation start.
 SUMMARY_OBSERVATION SEP_TEST_2008
 {
    VALUE    = 213;
    ERROR    =  10;
    DAYS     = 911;
    KEY      = GOPR:NESS;
 };



The BLOCK_OBSERVATION keyword 
------------------------------

This is observations of variables in grid blocks/cells. The
observations can be of arbitrary ECLIPSE fields like PRESSURE
(typically for an RFT), PORO or PERM. A block observation is entered
with the BLOCK_OBSERVATION keyword. Here is an example of a typical
block observation::

 BLOCK_OBSERVATION RFT_2006
 {
    FIELD = PRESSURE;
    DATE  = 22/10/2006;
 
    OBS P1 { I = 1;  J = 1;  K = 1;   VALUE = 100;  ERROR = 5; };
    OBS P2 { I = 2;  J = 2;  K = 1;   VALUE = 101;  ERROR = 5; };
    OBS P3 { I = 2;  J = 3;  K = 1;   VALUE = 102;  ERROR = 5; };
 };

This will condition on observations of the pressure in grid blocks
(1,1,1), (2,2,1) and (2,3,1) on the 22/10/2006.

By default the BLOCK_OBSERVATION requires that the specific field
which has been observed (e.g. PRESSURE in the example above) must have
been specified in main ERT configuration file using the FIELD keyword,
and ECLIPSE must be configured to produce a restart file for this
particular time. Alternatively it is possible to tell ERT to use the
summary vector as source of the data::

  BLOCK_OBSERVATION RFT_2006
  {
     FIELD = PRESSURE;
     DATE  = 22/10/2006;
     SOURCE = SUMMARY;  

     OBS P1 { I = 1;  J = 1;  K = 1;   VALUE = 100;  ERROR = 5; };
     OBS P2 { I = 2;  J = 2;  K = 1;   VALUE = 101;  ERROR = 5; };
     OBS P3 { I = 2;  J = 3;  K = 1;   VALUE = 102;  ERROR = 5; };
  };

In this case the data will be loaded from the BPR vectors in the
summary file.

Note the use of the sub class OBS to specify the actUal observed
values, the observation errors and their grid location. Each OBS shall
have a unique key within the BLOCK_OBSERVATION instance, and is
required to have the items I, J, K, VALUE and ERROR. These are the
grid i,j and k indicies for the observation point, the observed value
and it's standard deviation.

As with a SUMMARY_OBSERVATION, the observation time can be given as
either a date, days since simulation start or restart number. The
respective keys for setting giving it as date, days or restart number
are DATE, DAYS and RESTART. Note that each BLOCK_OBSERVATION instance
must have an unique global name (RFT_2006 in the example above).

Block observations can often be quite long. Thus, it is often a good
idea to use the special keyword include in order to store the OBS
structures in a different file. This is done as follows::

 BLOCK_OBSERVATION RFT_2006
 {
    FIELD   = PRESSURE;
    RESTART = 20;
    
    include 'RFT_2006_OBS_DATA.txt';  
 };

Where the file RFT_2006_OBS_DATA.txt contains the OBS instances::

   OBS P1 { I = 1;  J = 1;  K = 1;   VALUE = 100;  ERROR = 5; };
   OBS P2 { I = 2;  J = 2;  K = 1;   VALUE = 101;  ERROR = 5; };
   OBS P3 { I = 2;  J = 3;  K = 1;   VALUE = 112;  ERROR = 5; };
   OBS P4 { I = 3;  J = 3;  K = 1;   VALUE = 122;  ERROR = 5; };
   OBS P5 { I = 4;  J = 3;  K = 1;   VALUE = 112;  ERROR = 5; };
   OBS P6 { I = 5;  J = 3;  K = 1;   VALUE = 122;  ERROR = 5; };

   

The GENERAL_OBSERVATION keyword 
--------------------------------

The GENERAL_OBSERVATION keyword is used together with the GEN_DATA and
GEN_PARAM type. This pair of observation and data types are typically
used when you want to update something special which does not fit into
any of the predefined enkf types. The ERT application just treats
GENERAL_OBSERVATION (and also GEN_DATA) as a range of number with no
particular structure, this is very flexible, but of course also a bit
more complex to use::

 GENERAL_OBSERVATION GEN_OBS1{
    DATA     = SOME_FIELD;
    RESTART  = 20;
    OBS_FILE = some_file.txt;
 };

This example a minimum GENERAL_OBSERVATION. The keyword DATA points to
the GEN_DATA instance this observation is 'observing', RESTART gives
the report step when this observation is active. OBS_FILE should be
the name of a file with observation values, and the corresponding
uncertainties. The file with observations should just be a plain text
file with numbers in it, observations and corresponding uncertainties
interleaved. An example of an OBS_FILE::

 1.46 0.26
 25.0 5.0
 5.00 1.00

This OBS_FILE has three observations: 1.46 +/- 0.26, 25.0 +/- 5.0 and
5.00 +/- 1.00. In the example above it is assumed that the DATA
instance we are observing (i.e. comparing with) has the same number of
elements as the observation, i.e. three in this case. By using the
keywords INDEX_LIST or INDEX_FILE you can select the elements of the
GEN_DATA instance you are interested in. Consider for example::

 GENERAL_OBSERVATION GEN_OBS1{
    DATA       = SOME_FIELD;
    INDEX_LIST = 0,3,9; 
    RESTART    = 20;
    OBS_FILE   = some_file.txt;
 };

Here we use INDEX_LIST to indicate that we are interested in element
0,3 and 9 of the GEN_DATA instance::

 GEN_DATA                     GEN_OBS1
 ========                     ===========             
 1.56 <---------------------> 1.46  0.26
 23.0        /--------------> 25.0   5.00  
 56.0        |    /---------> 5.00  1.00
 27.0 <------/    |           =========== 
  0.2             |
 1.56             | 
 1.78             |
 6.78             |
 9.00             | 
 4.50 <-----------/
 ========

In addition to INDEX_LIST it is possible to use INDEX_FILE which
should just point at an plain text file with indexes (without any ','
or anything). Finally, if your observation only has one value, you can
embed it in the config object with VALUE and ERROR.

Matching GEN_OBS and GEN_DATA
.............................

It is important to match up the GEN_OBS observations with the
corresponding GEN_DATA simulation data correctly. The GEN_DATA result
files must have an embedded '%d' to indicate the report step in them -
in the case of smoother based workflows the actual numerical value
here is not important. To ensure that GEN_OBS and corresponding
GEN_DATA values match up correctly only the RESTART method is allowed
for GEN_OBS when specifying the time. So consider a setup like this::

 -- Config file:
 GEN_DATA RFT_BH67 INPUT_FORMAT:ASCII RESULT_FILE:rft_BH67_%d    REPORT_STEPS:20
 ...                                                       /|\                /|\ 
 ...                                                        |                  | 
 -- Observation file:                                       |                  |
 GENERAL_OBSERVATION GEN_OBS1{                              +------------------/ 
    DATA       = RFT_BH67;                                  | 
    RESTART    = 20;   <------------------------------------/
    OBS_FILE   = some_file.txt;
 };

Here we see that the observation is active at report step 20, and we
expect the forward model to create a file rft_BH67_20 in each
realization directory.  Error covariance

The optional keyword ERROR_COVAR can be used to point to an existing
file, containing an error covariance matrix. The file should contain
the elements of the matrix as formatted numbers; newline formatting is
allowed but not necessary. Since the matrix should by construction be
symmetric there is no difference between column-major and row-major
order! The covariance matrix

     [ 1      0.75  -0.25]
C =  [ 0.75   1.25  -0.50]  
     [-0.25  -0.50   0.85]

Can be represented by the file::

 1
 0.75
 -0.25
 0.75
 1.25
 -0.50
 -0.25
 -0.50
 0.85

without newlines, or alternatively::

 1       0.75  -0.25
 0.75    1.25  -0.50 
 -0.25  -0.50   0.85

with newlines. 
