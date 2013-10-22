## Working with 3D Views

3D Views are the windows displaying the Grid Models. The visualization is controlled by the **Project Tree** item representing the **View** and their subitems. 

![](images/3DViewOverview.png)


### Cell Results

The color of the cells in a grid model can be assigned using the Eclipse Property value for the cell. The property can be chosen in the property panel of the **Cell Result**. The property values are assigned a color according to the **Legend definition** 

### Cell Edge Results
When working with a reservoir model, it is useful to map a set of grid cell properties in the same view. This can be achieved by activating **Cell Edge Result**. When selecting a result variable for cell edge, a second legend shows up in the 3D view in addition to different color mapping along the edges of a cell. Color legend management is available when selecting **Legend Definition**.

### Cell Filters
A cell filter can be used to control visibility of a cell in the 3D view. Three types of filters exists
- Range filter : Define a IJK subset of the model
- Property filter : Define a value range for a property to control cell visibility
- Well cell filter : Display grid cells where a well is passing through

All filters can be controlled from the **Property Editor**.

#### Range filters

Using range filters enables the user to define a set of IJK visible regions in the 3D view. A single IJK-slice can be added from the context menu when rightclicking a grid cell. A new range filter can also be added by activating the context menu for the **Range Filters** collection in the **Project Tree**.

#### Property filters

In addition to range filters, it is possible to filter visible grid cells based on a property value range. Add a new property filter by activating the context menu for **Property Filters**. The new property filter is based on currently selected cell result.


### Wells

#### Well range filter 
Select **Wells** in the **Project Tree**. In the **Property Editor**, select **On** for **Add cells to range filter**. This will hide cells not part of a well.
In addition, all cells along a direction can be added as a fence. Enable this by checking **Use well fence**.
