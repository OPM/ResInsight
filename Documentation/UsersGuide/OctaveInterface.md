## Script interface to Octave #
ResInsight provides a flexible interface to [Octave](http://www.gnu.org/software/octave/ "Octave") for scripts execution. ResInsight can create an external Octave process, and is able to send and receive data from this external process.

### Script execution - single case 
A script can be started by navigating to the script in the **Project Tree**, and selecting **Execute** from the context menu. The currently active case will be manipulated by the Octave script. It is also possible to execute a script from the context menu of a case.

### Script execution - multiple cases 
One script can be executed on many cases by first selecting a set of cases, and then activating **Execute script** from the context menu for the case selection.

### Script management 
Octave scripts are available in the **Scripts** folder in the **Project Tree**. Multiple script folder locations can be defined in the field _Shared Script Folder(s)_ in **Edit=>Preferences**. These scripts can be edited by a text editor defined in _Script Editor_ in **Edit=>Preferences**.
