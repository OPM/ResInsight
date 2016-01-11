---
layout: docs
title: Intersections
permalink: /docs/intersections/
published: true
---

![]({{ site.baseurl }}/images/IntersectionOverview.png)

Intersections are cross sections of the grid model vertically or horizontally along a curve. The curve can be either a Simulation Well, a Well Path or a user defined polyline.

Intersections are  organized in a folder named **Intersections** in a **View** as shown below.

![]({{ site.baseurl }}/images/IntersectionInTree.png)

A new general intersection can be created by activating ![]({{ site.baseurl }}/images/CrossSection16x16.png) **New Intersection** from the context menu of the **Intersections** item in the Project Tree.

<div class="note info">
To be able to see the intersections in the 3D view, the grid cells can be hidden by disabling the <b>Grids</b> item in the Project Tree or activating the <b>Hide Grid Cells</b> toolbar button.
</div>

The property panel of an Intersection is shown below:

 ![]({{ site.baseurl }}/images/IntersectionWellPath.png)

#### Intersection Options

Property       | Description
---------------|------------
Name           | Automatically created based on the item specifying the intersection. The user can customize the name by editing, but will be updated if you change the well or well path.
Intersecting Geometry | These options controls the curve to be used for the cross section, and depends on the type of intersection you choose.
Direction      | Horizontal or vertical intersection
Extent length  | Defines how far an intersection for Well Path or Simulation Well is extended at intersection ends
Inactive cells | Controls if inactive cells are visualized on the intersection geometry


### Well Path Intersection
A new **Well Path** intersection can be created by right-clicking the well path in the 3D view or in the **Project Tree**. 

When a well path intersection is created, the source well path can be changed by using the **Well Path** selection combo box in the **Property Editor**.

### Simulation Well Intersection
A new **Simulation Well** intersection can be created by right-clicking the simulation well in the 3D view or in the **Project Tree**.

 ![]({{ site.baseurl }}/images/IntersectionSimulationWellProperties.png)

When a simulation well intersection is created, the source simulation well can be changed by using the **Simulation Well** selection combo box in the **Property Editor**. 

If the well contains more than one branch, the intersection geometry will be created for the selected brach in the **Branch** combo box.

### Polyline Intersection
A new **Polyline** intersection can be created from the context menu in the 3D view. Then, by left-clicking on reservoir geometry, a polyline is created. The points are added to the point list in the **Property Editor**. 

![]({{ site.baseurl }}/images/IntersectionPolyline.png)

The background color of this list is set to light pink when adding points by picking in the 3D view is active. To finish adding points, click the button **Stop picking points** in the **Property Editor**. The background color
of the point list is then set to white. 

The points in the list can be deleted and edited using the keyboard.
To append more points by clicking in the 3D view, push the button **Start picking points** again.

The points in the list can be copied to clipboard using **CTRL-C** when keyboard focus is inside the point list. A new list of points can be pasted into the point list by using **CTRL-V**.



