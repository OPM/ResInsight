---
layout: docs
title: Derived Results
permalink: /docs/derivedresults/
published: true
---

ResInsight computes several derived results. In this section we will explain what they are, and briefly how they are calculated.

## Derived results for Eclipse cases

The derived results are listed at the bottom of the **Static** result properties, as shown below.

![]({{ site.baseurl }}/images/DerivedStaticResults.png)


###Transmissibility normalized by area
The transmissibility for cells and Non-Neighbor Connections (NNCs) are dependent on both cell properties and geometry. ResInsight normalizes TRANX, TRANY and TRANZ with the overlapping flow area for both neighbor cells and NNC-cells. The results are named riTRANXbyArea, riTRANYbyArea and riTRANZbyArea respectively.

The normalized transmissibilities make it easier to compare and check the flow capacity visually. This can be useful when history matching pressure differences across a fault. 

###Identification of questionable NNCs
In the process of normalizing transmissibility by the overlapping flow area, the NNCs in the model without any shared surface between two cells are identified. These NNCs are listed in the **Faults/NNCs With No Common Area** folder. These NNCs are questionable since flow normally is associated with a flow area.

![]({{ site.baseurl }}/images/ResInsight_NNCsWithNoCommonArea.png)
 

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

## Derived Geomechanical results

ResInsight calculates several of the presented geomechanical results based on the native results present in the odb-files. 

The calculated result fields are:

SE (Effective Stress) , E (Strain), ST (Total Stress) and Gamma (Stress path)

In this text the label Sa and Ea will be used to denote the unchanged stress and strain tensor respectivly from the odb file.

Components with one subscript denotes the principal values 1, 2, and 3 which refers to the maximum, middle, and minimum principals respectively. 

Components with two subscripts however, refers to the global directions 1, 2, and 3 which corresponds to  X, Y, and Z and thus also Easting, Northing, and -Depth.

### SE - Effective Stress

SE<sub>ij</sub> = -Sa<sub>ij</sub> (Where POR is defined) 

SE<sub>ij</sub> = *undefined* (Were POR is not defined)

SE<sub>i</sub> = Principal value i of SE

### E - Strain

E<sub>ij</sub> = -Ea<sub>ij</sub>

### ST - Total stress

ST<sub>ii</sub> = -Sa<sub>ii</sub> + POR (i= 1,2,3)

ST<sub>ij</sub> = -Sa<sub>ij</sub> (i,j = 1,2,3 and i not equal j)

We use a value of POR=0.0 where it is not defined.

ST<sub>i</sub> = Principal value i of ST

### Gamma - Stress path

Gamma<sub>ii</sub> = ST<sub>ii</sub>/POR (i= 1,2,3) 

Gamma<sub>i</sub> = ST<sub>i</sub>/POR 

In these calcualtioins we set Gamma to *undefined* if abs(POR) > 0.01 MPa. 
