---
layout: docs
title: Completions LGR
permalink: /docs/completionslgr/
published: true
---
![]({{ site.baseurl }}/images/TemporaryLGR_CompletionType.png) ![]({{ site.baseurl }}/images/TemporaryLGR_CompletionType2.png)

ResInsight supports creating and exporting LGRs (Local Grid Refinement) in main grid cells intersected by completions.

## Create Temporary LGRs
To create temporary LGR(s), first select one or more well paths in the project tree, right click and select 'Create Temporary LGR'. This command brings up the following dialog.

![]({{ site.baseurl }}/images/TemporaryLgrDialog.png)

The dialog contains several input fields
- **Source Case** -- The source case/grid
- **Time Step** -- The time step to use
- **Fractures, Fishbones, Perforations** -- Check boxes to control which completion types to create LGRs for
- **Split Type** -- Three options controlling how the LGRs will be created
  - **LGR Per Cell** -- One LGR is created for each main grid cell intersected by completions of the selected type
  - **LGR Per Completion** -- One LGR is created for each completion of the selected types. Each LGR span all main grid cells that are located within an IJK bounding box containing all intersected cells.
  - **LGR Per Well** -- One LGR is created for each of the selected well paths. The LGR spans all main grid cells that are located within an IJK bounding box containing all intersected cells for all selected completions on that well path.
- **Cell Count I,J,K** -- The size of the LGR, in each main grid cell, in the I, J and K direction

After pressing the OK button, LGR(s) are created and will be visible in the grid view.
Temporary LGRs are stored in memory, and are thus not saved to file. After restarting ResInsight those LGRs have to be recreated.

Individual visibility of generated LGRs can be controlled from the [Grids]({{ site.baseurl }}/docs/reservoirviews/#grids-) section in a view.

### Delete temporary LGRs
It is possible to explicitly delete all temporary LGRs. Right click on the **View -> Grids -> Temporary LGRs** project tree node and select **Delete Temporary LGRs**. This command deletes all temporary LGRs.
