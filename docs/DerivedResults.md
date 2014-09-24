##Derived Results

###Transmissibility normalized by area
The transmissibility for cells and Non-Neighbor Connections (NNCs) are dependent on both cell properties and geometry. ResInsight normalizes TRANX, TRANY and TRANZ with the overlapping flow area for both neighbor cells and NNC-cells. The results are named riTRANXbyArea, riTRANYbyArea and riTRANZbyArea respectively and are listed under **Static** properties. 

The normalized transmissibilities make it easier to compare and check the flow capacity visually. This can be useful when history matching pressure differences across a fault. 

###Overall transmissibility multiplyer
Transmissibility can be set or adjusted with multiple keywords in an Eclipse data deck. To visualize the adjustments made, ResInsight calculates a multiplicator for the overall change. First unadjusted transmissibilities for all neighbor cells and NNCs are evaluated based on geometry and permeabilities, similar to the NEWTRAN algorithm in Eclipse. For x- and y-directions, the NTG parameter is also included. The results are named riTRANX, riTRANY and riTRANZ respectively.

The TRANX, TRANY and TRANZ used in the simulation are divided by the ResInsight calculated transmissibilities and the resulting multiplicators are named riMULTX, riMULTY and riMULTZ respectively. The derived properties are listed under **Static** properties. The riMULT-properties are useful for quality checking consistence in user input for fault seal along a fault plane. 


###Directional combined results
Some static properties with directional dependency can be visualized in x-, y- and z-direction combined in **Cell Result** and **Separate Fault Result**. The face of a cell is then colored based on the value associated with that particular face. The Positive I-face of the cell gets the cell X-value, while the J-face gets the Y-value etc. The negative faces, however, get the value from the neighbor cell on that side. The negative I-face gets the X-value of the IJK-neighbor in negative I direction, and so on for the J- and K-faces.

The directional combined parameters available are:
- **TRANXYZ** (inluding NNCs)
- **MULTXYZ**
- **riTRANXYZ** (inluding NNCs)
- **riMULTXYZ** (inluding NNCs)
- **riTRANXYZbyArea** (inluding NNCs)
