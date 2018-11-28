---
layout: docs
title: Create Well Paths
permalink: /docs/createwellpaths/
published: true
---
![]({{ site.baseurl }}/images/WellPathCreation.png)

ResInsight lets the user create new/custom well paths by clicking in the 3D view. A self created well path will behave the same way as an ordinary imported well path.

To create a well path:
1. Right click **Wells** in the project tree
2. Select **Create Well Path** in the context menu. A new well node and a well targets node are created.
3. Click in the 3D view on locations where the well path will pass (targets). Note. A 3D object must be hit when clicking. Clicking in thin air will not work.
4. When finished placing targets, click on "Stop Picking Targets" in the property editor

![]({{ site.baseurl }}/images/WellTargetsTree.png)

![]({{ site.baseurl }}/images/WellTargetsPropertyEditor.png)

Well targets property editor fields
- **UTM Reference Point** - Reference point. Defaults to the first target point clicked
- **MDRKB at First Target** - Define MD (referenced to Rotary Kelly Bushing) at first target point. Applies to well path export only.
- **Well Targets:** List of targets. Will have pink background when in picking state.
  - **Point** - Target position relative to reference point
  - **DL in** - Dog leg inwards. Unit: Degrees/30m
  - **DL out** - Dog leg outwards. Unit: Degrees/30m
  - **Dir** - Check box for overriding well path auto calculated directions
  - **Azi (deg)** - Azimuth. Y axis is 0 degrees
  - **Inc (deg)** - Inclination. Z axis is 0 degrees

A self created well path may be edited by either editing coordinates in the property editor or clicking and dragging targets in the 3D view. 

![]({{ site.baseurl }}/images/WellTargets.png)

Clicking and dragging the blue part of a target, it can be moved along the Z axis only. Clicking and dragging the magenta part of a target, it can be moved freely around.

### Well Plan
A well plan can be displayed by selecting **Show Well Plan** from the context menu of a generated well path. 

```
-- MDRKB          CL            Inc          Azi            TVDMSL         NS                EW               Dogleg      Build        Turn          
0.00000        0.00000       91.23218     67.46458       2560.64104     7320830.60875     456093.37763     0.00000     0.00000      0.00000       
109.69761      109.69761     91.23218     67.46458       2558.28210     7320872.64113     456194.67560     0.00000     0.00000      0.00000       
383.74602      274.04841     90.01335     94.84447       2555.24550     7320914.37090     456462.87518     3.00000     -0.13343     2.99727       
518.95310      135.20708     88.73259     106.92696      2556.73062     7320888.88803     456595.39303     2.69569     -0.28418     2.68089       
597.83872      78.88561      88.73259     106.92696      2558.47547     7320865.92593     456670.84259     0.00000     0.00000      0.00000       
730.04479      132.20607     90.46303     138.71821      2559.42803     7320795.18191     456780.51358     7.22424     0.39267      7.21402       
1087.97847     357.93368     92.23380     -164.19586     2550.23230     7320460.45997     456856.25443     4.78528     0.14842      -25.38856     
```
