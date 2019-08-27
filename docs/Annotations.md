---
layout: docs
title: Annotations
permalink: /docs/annotations/
published: true
---

ResInsight supports displaying a few types of annotations in 3D views and Contour Map view.
- Text annotations
- Reach circle annotations
- Polyline annotations
  - User defined polylines
  - Polylines imported from file

![]({{ site.baseurl }}/images/Annotations.png)

## Global scope vs local scope annotations
Global annotations may be displayed in all views and are located in the **Annotations** project tree node right below **Grid Models** (Global annotations sub tree). Local annotations are associated with a specific view and are located in the **Annotations** project tree node below the view node (Local annotations sub tree). All annotation types except text annotations are global only. Text annotation may be either global or local.

All global annotations also have a representation in the local **Annotation** tree node in order to toggle visibilty per view. Those annotations are located in tree nodes starting with **Global**.

![]({{ site.baseurl }}/images/LocalAnnotationsTree.png)<br/>
Local annotations sub tree

![]({{ site.baseurl }}/images/GlobalAnnotationsTree.png)<br/>
Global annotations sub tree

## Text Annotations
There are two ways of creating a new text annotation.
- Right click **Annotations** or **Text Annotations** tree node in either the global annotatyion sub tree or the local annotations sub tree. The scope of the annotation depends on which node was clicked. When text annotations are created this way, all text annotation fields must be entered manunally in the property editor.
- Right click on an object in the view and select **Create Text Annotation**. ResInsight will then create a text annotation at the clicked point. In this case, only th text must be entered manually in the property editor. When creating a text annotation this way, it will become a local annotation by default.

![]({{ site.baseurl }}/images/TextAnnotationPropertyEditor.png)

- **Anchor Point** - The interesting point in the view
- **Label Point** - The point where the text label is placed
- **Text** - The text to display. Multiline supprted. The first line will be the name of the annotation in the project tree
- **Text appearance** - Set font size, font color, background color and anchor line color

When a text annotation tree node is selected, target markers in each end of the anchor line are displayed. The targets can be clicked and dragged. Clicking the blue part lets the user drag the target vertically (along Z axis). Clicking the magenta part lets the user drag the target in the XY plane.

## Reach Circle Annotations
To create a reach circle annotation, right click **Annotations** or **Reach Circle Annotations** tree node in the global annotations sub tree. Then enter values in the property editor.

![]({{ site.baseurl }}/images/CircleAnnotationPropertyEditor.png)

- **Name** - Name of the circle annotation
- **Center Point** - Center point of the circle. Format 'x-pos y-pos depth'
- **Radius** - Circle radius
- **Line Appearance** - Set circle color and line thickness

## User Defined Polyline Annotations
To create a user defined polyline annotation, right click **Annotations** or **User Defined Polyline Annotations** tree node in the global annotations sub tree. The property editor for the newly created annotation is displayed and is in picking points mode. The user may now click on objects in the view to create polyline points. When finished, click **Stop Picking Points** in the property editor.

![]({{ site.baseurl }}/images/UserDefinedPolylineAnnotationPropertyEditor.png)

- **Targets** - Polyline points. Can be edited in the table or interactively in the view
- **Start Picking Points / Stop Picking Points** - Button to start / stop picking mode
- **Line Appearance** - Set line color and thickness

When a user defined polyline annotation tree node is selected, the polyline target markers become visible. Those can be dragged around as decribed above.

## Polyline Imported From File
To import a polyline annotation from file, right click **Annotations** or **Polylines From File** tree node in the global annotations sub tree. Then select the file to import and click OK. Imported polyline annotations are not editable.

![]({{ site.baseurl }}/images/PolylineFromFileAnnotationPropertyEditor.png)

- **File** - Name of the imported file
- **Line Appearance** - Set line color and line thickness

## Annotations visibility
Local annotations visibility is controlled by the check boxes in the local annotations sub tree only. Global annotations visibility, on the other hand, is controlled by the check boxes in both the global and local annotations sub trees. So in order to display a global annotation in a specific view, both the annotation tree note itself and its representation in the local sub tree must have visibilty enabled.


