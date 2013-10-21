
## Multiple realizations and statistics

ResInsight features efficient support for management of large collection of realizations. 
To import a set of realizations, select **File->Import->Create Grid Case Group from Files**.
An import dialog is opened, an a set of cases can be imported from multiple folders.

*NOTE:* The search for Eclipse cases will be executed recursively for the specified folders.

Clicking the **Ok** button will import all cases into ResInsight, and they can be inspected one by one. To reduce the number of views, only a view for the first case is automatically created. Select *New view** from the context menu of a case to create a 3D view of the case.

### Statistics ##
After creating a grid case group, an empty statistics object is created. Select the properties to evaluate statistics for, and push **Compute** to start the computation of statistics. This can take a while for large models.
When the computation is complete, a view is automatically created containing the resulting generated statistics properties. Interaction with these generated properties are identical to interaction with other properties read from file.

A new statistical calculation can be created by activating the context menu for **Derived Statistic->New Statistics Case**.
