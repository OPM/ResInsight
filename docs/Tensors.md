---
layout: docs
title: Tensor Results
permalink: /docs/tensors/
published: true
---

![]({{ site.baseurl }}/images/tensorArrows.png)

Tensors are arrows showing the average principal vectors in an element, shown on every visible face of the element.

The tensor results editor is found in a geo mechanical model's **View** in the project tree as seen below.

![]({{ site.baseurl }}/images/tensorProjectTree.png)

## Visualization

![]({{ site.baseurl }}/images/tensorPressureTension.png)

The tensor arrows visualize the principal vectors in three directions. Each colored pair of arrows represents a principal.
In the example above, the orange and blue arrows represent pressures and the white arrows represent a tension. 

## Properties

![]({{ site.baseurl }}/images/tensorPropEditor.png)

### Value
Tensor Results of an element can be calculated from one of the three result values *SE, ST* and *E*.

### Visibility
Choose which of the three principals to be shown. The threshold removes all principals with an absolute value less than or equal to the threshold value.

### Vector Colors
Choose which color palette to use for the three arrows. The colors appear in "correct" order (first color = principal 1). 

The vector color **Result Colors** is special. By choosing this color type, a new legend will appear. This legend is defined by the values in the Legend definition of the Element Tensor Results. The extreme values of the color mapper are the extremes of the three principals combined. In the example below, the color result is SE-S1. The largest arrow (principal 1) is quite similar to the cell color, as expected.

![]({{ site.baseurl }}/images/tensorsResultColor.png)

### Vector Size
Scale method **Result** scales the arrows relative to the maximum result value of all components in the model. With scale method **Constant**, all the arrows are set to an equal constant size. The overall arrow size can be adjusted by using the **Size Scale**.

