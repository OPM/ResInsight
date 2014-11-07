---
layout: docs
prev_section: modelnavigation
next_section: buildinstructions
title: Preferences
permalink: /docs/preferences/
published: true
---

User defined preferences are store for each user of ResInsight.

![ResInsight User Interface]({{ site.baseurl }}/images/Preferences.png)

## Configuration

- **Navigation mode** Defines how to use the mouse to interact with with the 3D model.
- **Script configuration** Defines where scripts are stored and what text editor to use for editing scripts
- **Octave** Defines the binary file of octave. Usually without path on Linux, and including path on Windows.
-  **Default settings** The user can define a set of visual settings that will be used when creating new views.
-  **Behaviour when loading new case**
    - **Compute when loading new case** If not present, compute DEPTH, DX, DY, DZ, TOP, BOTTOM when loading new case
    - **Load and show SOIL** Control if SOIL is loaded and applied to grid
-  **Reader settings** Defines entities to import in addition to grid and grid properties.
-  **ssihub Address** Statoil internal web service used to import well paths.
-  **Use shaders** In some settings hardware accelerated methods are not possible to use(ie. remote desktop on Windows). This setting can disable shaders to be able to test behaviour without shaders.
-  **Show 3D Information** Displays resource usage from the graphics card.

