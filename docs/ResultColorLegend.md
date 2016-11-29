---
layout: docs
title: Result Color Legend
permalink: /docs/resultcolorlegend/
published: true
---

The color mapping of the displayed cell result is controlled by the **Legend Definition** located below a result node in the **Project Tree**

![Legend Configuration]({{ site.baseurl }}/images/legend_configuration.PNG)


Item               | Description
-------------------|------------
Number of levels   | Defines the number of tickmarks displayed next to the color legend
Significant digits | Defines the number of significant digits in the number formatting
Number format      | Defines how the numbers are formatted
Colors             | Defines the color palette

## Mapping
- **Discrete Linear** - Legend divided into levels defined by **Number of levels**
- **Continuous Linear** - Continuous legend with tickmark count defined by **Number of levels**
- **Continuous Logarithmic** - Continuous logarithmic legend with tickmark count defined by **Number of levels**
- **Discrete Logarithmic** - Logarithmic legend divided into levels defined by **Number of levels**
- **Category** - Special legend with one level for each category, either integer values or formation names. Only available for result names ending with ```NUM``` or formation names.


## Range type
- **All Timesteps** - values for all time steps are used to find min and max value of the result values represented by the color legend
- **Current Time Step** - use current (one) time step to find min and max values  
- **User Defined Range** - user specifies numeric values for min and max  
