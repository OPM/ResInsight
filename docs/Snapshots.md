---
layout: docs
title: Snapshots
permalink: /docs/snapshots/
published: true
---

ResInsight can take screen shots of your different 3D Views and Plot Windows directly. These commands are available from the toolbar and the menues in the respective main window.  

![]({{ site.baseurl }}/images/SnapShotToolBar.png)

### Snapshot to Clipboard ![]({{ site.baseurl }}/images/SnapShot.png)

A snapshot of the active view is copied to the clipboard using **Edit -> Copy Snapshot To Clipboard**.

### Snapshot to File ![]({{ site.baseurl }}/images/SnapShotSave.png)

Image export of the currently active 3D View or Plot Window can be launched from **File -> Export -> Snapshot To File**. 

### Snapshot All Views/Plots to File ![]({{ site.baseurl }}/images/SnapShotSaveViews.png)

If a project contains multiple 3D Views or Plot Windows, all of them can be exported in one go using **File -> Export -> Snapshot All Views To File**. This will either export all the 3D Views or all the Plot Windows, depending on whether you invoke the command in the 3D Main Window or the Plot Main Window.

The files generated are stored in a folder named `snapshots` within the folder where the Project File resides. 

<div class="note">
 Snapshots can also be created and saved from the command line. 
 (See <a href="{{ site.baseurl }}/docs/commandlineparameters">Command Line Arguments</a> )
</div>
