---
layout: docs
title: Export Completions
permalink: /docs/completionexport/
published: true
---

The completions defined in ResInsight can be exported to Eclipse for use in new simulation runs. The commands **Export Completion Data For Visible Wells**, **Export Completion Data For Visible Simulation Wells** and **Export Completion Data For Selected Wells** can be used to invoke the export. The commands are available by right clicking Well Paths or Simulation Wells in the **Project Tree**. The first command is available from the **File->Import** menu as well.

![]({{ site.baseurl }}/images/Completions_ExportCompletionData.png)

- **File Settings**
  - **Export Folder** -- Folder for the exported COMPDAT file(s). The folder will be created when performing the export and the names of the exported file(s) will be auto generated.
  - **File Split** -- Controls how ResInsight splits the export in different files
    - **Unified File** -- One file with all the completions. 
    - **Split on Well** -- One file for each well
    - **Split on Well and Completion Type** -- One file for each well and completion type (Perforation Interval, Fishbone, ...)
  - **Export Completion Types** -- Control how several completions of different type in the same cell are handled
    - **Individually** -- Completions of each type are exported to separate sections in the file and not combined in any way.  
    - **Combined** -- Connection factors from different completion types are added together producing one number for each cell. 
- **Settings**  
  - **Case to Apply** -- Select which case to use for the export. Matrix transmissibilities will be read from this case.  
  - **Export** -- 
    - **Calculated Transmissibilities** -- The transmissibilities calculated based on the case and completion data are exported directly
    - **Default Connection Factors and WPIMULT** -- The information about the connections for Eclipse to be able to make the transmissibility calculaton is exported for the COMPDAT keyword. In addition, the same transmissibility calculation is performed by ResInsight, and the factor between the actual transmissibility for the connection and the Eclipse calculation is exported in the WPIMULT keyword. 
  - **Use NTG Horizontally** -- Toggles whether NTG in I and J directions is included in the calculation
- **Visible Completions**
  - **Perforations** -- Option to include or exclude perforation intervals in the export. 
    - **Time step** -- Which timestep to export. This option is included since perforation intervals have a start time, and thus not all perforations need be present at all time steps. 
  - **Fractures** -- Option to include or exclude fracture completions from the export.
  - **Fishbones** -- Option to include or exclude fishbone completions from the export. 
    - **Exclude Main Bore Transmissibility** -- If this options is checked on, only the transmissibilities for the fishbone laterals will be included in the export, and transmissibility along the main bore will not contribute. 

### Transmissibility Calculations

The transmissibility calculation is performed for each direction, X, Y and Z, in an orthogonal coordinate system local to the cell. 

Taking the X direction as an example, we first calculate the relevant permeability *K* from the Eclipse properties *PERMY* (K<sub></sub>) and PERMZ (K<sub>z</sub>): 

![]({{ site.baseurl }}/images/Equation_PerfInterval_K.png)

The Peacman radius (pressure equivalent radius) for the cell is then calculated, using permeabilities and cell sizes (D<sub>y</sub> and D<sub>z</sub>): 

![]({{ site.baseurl }}/images/Equation_PerfInterval_Peaceman.png)

The x-component of the transmissibility vector is calculated, using the length of the perforation in the x direction (l<sub>x</sub>), the well radius (r<sub>w</sub>) and skin factor (S):

![]({{ site.baseurl }}/images/Equation_PerfInterval_Trans.png)

The y and z component of the transmissibility are calculated in the same manner, and the total transmissibility is then calculated as: 

![]({{ site.baseurl }}/images/Equation_PerfInterval_TotalT.png)

If the *Export Calculated Transmissibilities* is chosen in the export setting (see [Exporting Completion Data to Eclipse](#exporting-completion-data-to-eclipse)), this value is exported in the COMPDAT keyword directly. If the *Export Default Connection Factors and WPIMULT* the transmissibility is chosen, the transmissibility is calculated as above, and in addition the transmissibility is calculated as Eclipse would do it using values other than transmissibility in the COMPDAT keyword (perforation length, well radius etc). The ratio between these transmissibilities is then exported as the WPIMULT value. 

For an example of *COMPDAT* files exported with calculated transmissibilities and with defaults and WPIMULT values, see export of fishbones completion data below.  

### Export of Fishbone Completion Data

The transmissibility calculation for the fishbones is done following the above description except that when calculating the transmissibility for the laterals, the full cell volume is split among the laterals for calculation of the transmissibility. This is done by finding the direction of the main bore, and then dividing the cell size in this direction by the number of laterals in the cell when calculating the Peaceman radius. 

An example of the exported COMPDAT file is shown below. The calculated transmissibility contribution to the cell connection factor from each lateral or main bore part is included as a comment. 

    COMPDAT
    -- Well            I      J      K1     K2     Status     SAT     TR               DIAM     KH     S     Df     DIR     r0     
    -- Well Path B main bore : 0.0569986
       Well Path B     26     45     29     29     OPEN       1*      5.699858E-02      /
    -- Fishbone 0: Sub: 0 Lateral: 0 : 0.0021382
    -- Fishbone 0: Sub: 0 Lateral: 1 : 0.00228575
    -- Fishbone 0: Sub: 0 Lateral: 2 : 0.0126269
    -- Fishbone 0: Sub: 1 Lateral: 1 : 0.0112929
    -- Fishbone 0: Sub: 2 Lateral: 0 : 0.00566964
    -- Well Path B main bore : 0.230572
       Well Path B     27     41     15     15     OPEN       1*      2.645858E-01      /
    /
    

For export with WPIMULT factors, the main bore diameter and direction are given in the export for cells which have both main bore and lateral contributions, while diameter and main direction of the first lateral is used for cells with no main bore contribution. Other parameters exported as part of COMPDAT are set to default. 

The *WPIMULT* parameters are calculated, as for the perforation intervals, by ResInsight calculating both the transmissibility of the completion as described above, and in addition calculating the transmissibility based on the information exported in the COMPDAT keyword. The ratio between these two numbers is then exported as the *WPIMUT* keyword. 

    COMPDAT
    -- Well            I      J      K1     K2     Status     SAT     TR     DIAM        KH     S           Df     DIR     r0     
    -- Well Path B main bore : 0.0569986
       Well Path B     26     45     29     29     OPEN       1*      1*     0.21600     1*     0.00000     1*     'Z'      /
    -- Fishbone 0: Sub: 0 Lateral: 0 : 0.0021382
    -- Fishbone 0: Sub: 0 Lateral: 1 : 0.00228575
    -- Fishbone 0: Sub: 0 Lateral: 2 : 0.0126269
    -- Fishbone 0: Sub: 1 Lateral: 1 : 0.0112929
    -- Fishbone 0: Sub: 2 Lateral: 0 : 0.00566964
    -- Well Path B main bore : 0.230572
       Well Path B     27     41     15     15     OPEN       1*      1*     0.21600     1*     0.00000     1*     'Z'      /
    /
    WPIMULT
    -- Well            Mult         I      J      K      
       Well Path B     0.70133      25     45     29      /
       Well Path B     25.11396     27     41     15      /
    /



## Export Well Segments

It is possible to export all the Fishbone Subs Definitions to a text file containing the Eclipse input data 
keywords needed to represent the fishbone part of the well as an MSW.

This can be done by the command **Export Well Segments** available as a context command on the **Fishbones** folder. Invoking the command will show a dialog prompting you to enter a target directory and which case to use in the calculations.

![]({{ site.baseurl }}/images/Fishbones_ExportWellSegments.png)

### Exported MSW Data

In the output file there are data for three Eclipse keyword specified.

##### WELSEGS
WELSEGS defines multi-segment well. The list of entries contains information on the main stem, the ICDs at the fishbone subs and the fishbone laterals. A comment above each entry details which element (main bore / ICD / lateral) the entry is for.  Example: 

    WELSEGS
    -- Name            Dep 1          Tlen 1       Vol 1     Len&Dep     PresDrop     
       Well Path A     4137.09154     87.00000     1*        ABS         H--           /
    -- First Seg     Last Seg     Branch Num     Outlet Seg     Length        Depth Change     Diam        Rough       
    -- Main stem
    -- Segment for sub 0
       2             2            1              1              13.00000      0.53667          0.15200     0.00001      /
    -- Laterals
    -- Diam: MSW - Tubing Radius
    -- Rough: MSW - Open Hole Roughness Factor
    -- ICD
       3             3            2              2              0.10000       0                0.15200     0.00001      /
    -- Fishbone 0 : Sub index 0 - Lateral 0
       52            52           27             3              1.70326       -0.57276         0.00960     0.00100      /
       53            53           27             52             2.34748       -0.81635         0.00960     0.00100      /
    /
   

- The first *WELSEGS* entry contains information about the well: 
  - **Name** - Name of well
  - **Dep 1** - TVD of start MD point, as given by the user in the Fishbones **Start MD** field.  
  - **Tlen 1** - Point given by the user in the Fishbones **Start MD** field.  
  - **Len&Dep** - incremental or absolute, as specified by the user in the Fishbones property editor. 
  - **PresDrop** - specifies what is included in the pressure drop calculation, hydrostatic, friction or acceleration. Specified by user in the Fishbones property editor.

- The following *WELSEGS* entries contains information about each segment: 
  - **First Seg**, **Last Seg** -- Values are being exported pr segment, so both first and last segment number is the number of the segment being exported. 
  - **Branch Num** -- Branch number for segment being exported.
  - **Outlet Seg** -- The segment the exported segment is connected to. For the main bore segments, this is the segment before them, for ICDs the segment number being exported and for fishbone laterals the segment on the main broe where the laterals are connected.  
  - **Length** -- Length of segment (if incremental Len&Dep above) or length of segments including this along well (if absolute Len&Dep above). For ICDs length is set to 0.1. 
  - **Depth Change** -- Depth of segment, incremental or absolute as for Length. For ICDs depth is set to 0. 
  - **Diam** -- Diameter of segment. For main bore and ICD entries, the liner inner diameter for the Fishbones collection is used. For laterals, an effective diameter is calculated so that the diameter exported is the diameter which, assuming a circle, would give the same area as the area between the hole diameter and the tubing diameter.  
  - **Rough** -- The roughness factor as entered by the user. Notice that a different value can be specified for the main bore and the laterals, as described above.       

    
##### COMPSEGS
An example of the COMPSEGS keyword as exported is shown below.  

    COMPSEGS
    -- Name            
       Well Path A      /
    -- I      J      K      Branch no     Start Length     End Length     Dir Pen     End Range     Connection Depth     
       28     40     6      27            0.00000          1.70326         /
       28     40     7      27            1.70326          2.34748         /
       28     40     8      27            2.34748          2.96577         /
    /

The first COMPSEGS entry is a line with the well path name. Each following entry is for the segments in the well, and containing the following field: 
- **I**, **J** and **K** -- The Eclipse cell index
- **Branch no** -- Branch number for the segment
- **Start Length**, **End Length** -- Start and end length along the well for the relevant segment. 

##### WSEGVALV
An example of the WSEGVALV keyword as exported is shown below.  

    WSEGVALV
    -- Well Name       Seg No     Cv          Ac          
       Well Path A     3          1.50000     0.00008      /
       Well Path A     5          1.50000     0.00008      /
       Well Path A     7          1.50000     0.00008      /
    /
    
The parameters exported in the WEGVALV keyword are
- **Well Name** -- The name of the well.
- **Seg No** -- Segment number along the well.
- **Cv** -- The ICD Flow Coefficient, as entered by the user.
- **Ac** -- the total ICD area per sub, calculated as the area per ICD (given by the orifice radius) multiplied with the number of icd per Sub.  
    
