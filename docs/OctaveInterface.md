---
layout: docs
prev_section: casegroupsandstatistics
next_section: wellpaths
title: Octave Interface
permalink: /docs/octaveinterface/
published: true
---

ResInsight provides a flexible interface to [Octave](http://www.gnu.org/software/octave/ "Octave").
This includes a set of Octave functions that communicates with a running ResInsight session, features in ResInsight that makes it easy to manage and edit Octave scripts, and their execution using Octave.  

The Octave functions are documented in [Octave Interface Reference]({{ site.baseurl }}/docs/octaveinterfacereference).

## Script management 
Octave scripts are available in the **Scripts** folder in the **Project Tree**. 

![]({{ site.baseurl }}/images/OctaveScriptTree.png)

This folder contains an entry for each of the directories you have added as a **Script Folder**. Each of the folder lists available `*.m` files and sub directories. The tree is continuously updated to reflect the file structure on disk.

### Adding Script Folders
You can add directories by right clicking the **Scripts** item to access the context menu.

Multiple standard script folder locations can also be defined in the field **Shared Script Folder(s)** in the **Preferences Dialog** (**Edit -> Preferences**). 

### Editing scripts 
To enable script editing from ResInsight you need to set up the path to a text editor in the **Script Editor** field in the **Preferences Dialog** (**Edit -> Preferences**) 

When done, scripts can be edited using the context menu command **Edit** on the script item in the tree.

## Script execution
Octave scripts can be executed with or without a selection of cases as context. The [Octave Interface Reference]({{ site.baseurl }}/docs/octaveinterfacereference) highlights in more depth how to design your Octave scripts to utilize these features.

### Without a case selection 
A script can be started by navigating to the script in the **Project Tree**, and selecting **Execute** from the context menu. The currently active case (The one with the active 3D View) will then be set as ResInsight's *Current Case*. 

### With a case selection
One script can be executed on many cases by first selecting a set of cases, and then activating **Execute script** from the context menu for the case selection. The script is then executed once pr selected case. Each time ResInsight's *Current Case* is updated, making it accessible from the Octave script. 

![]({{ site.baseurl }}/images/ExecuteOctaveScriptOnSelectedCases.png)