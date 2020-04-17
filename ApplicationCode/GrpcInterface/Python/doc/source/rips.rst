Instance Module
===============

.. autoclass:: rips.instance.Instance
    :members:

Example
-------

.. literalinclude:: ../../rips/PythonExamples/instance_example.py
	:language: python
	:lines: 5-
	:emphasize-lines: 3
	
Case Module
===========
.. autoclass:: rips.case.Case
    :members:

Example
-------
	
.. literalinclude:: ../../rips/PythonExamples/all_cases.py
	:language: python
	:lines: 5-
	:emphasize-lines: 5
	
Contour Map Module
==================

.. autoclass:: rips.contour_map.EclipseContourMap
    :members:

.. autoclass:: rips.contour_map.GeoMechContourMap
    :members:

Grid Module
===========

.. autoclass:: rips.grid.Grid
    :members:

Example
-------
.. code-block:: python

	case = rips_instance.project.loadCase(path=casePath)
    print (case.gridCount())
	if case.gridCount() > 0:
		grid = case.grid(index=0)
		dimensions = grid.dimensions()
		print(dimensions.i)
		print(dimensions.j)
		print(dimensions.k)

GridCaseGroup Module
====================

.. autoclass:: rips.gridcasegroup.GridCaseGroup
    :members:


Plot Module
==============

.. autoclass:: rips.project.Plot
    :members:

Project Module
==============

.. autoclass:: rips.project.Project
    :members:


Simulation Well Module
======================

.. autoclass:: rips.simulation_well.SimulationWell
    :members:

View Module
===========

.. autoclass:: rips.view.View
    :members:

Well Log Plot Module
====================

.. autoclass:: rips.well_log_plot.WellLogPlot
    :members:
    
Synchronous Example
-------------------
Read two properties, multiply them together and push the results back to ResInsight in a naïve way, by reading PORO into a list, then reading PERMX into a list, then multiplying them both in a resulting list and finally transferring back the list.

This is slow and inefficient, but works.

.. literalinclude:: ../../rips/PythonExamples/input_prop_test_async.py
	:language: python
	:lines: 5-
	
Asynchronous Example
--------------------
Read two properties at the same time chunk by chunk, multiply each chunk together and start transferring the result back to ResInsight as soon as the chunk is finished.

This is far more efficient.
	
.. literalinclude:: ../../rips/PythonExamples/input_prop_test_sync.py
	:language: python
	:lines: 5-
	
