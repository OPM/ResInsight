Instance Module
===============

.. autoclass:: rips.Instance
    :members:

Example
-------

.. literalinclude:: ../../rips/PythonExamples/InstanceExample.py
	:language: python
	:lines: 5-
	:emphasize-lines: 3
	
Case Module
===========
.. autoclass:: rips.Case
    :members:

Example
-------
	
.. literalinclude:: ../../rips/PythonExamples/AllCases.py
	:language: python
	:lines: 5-
	:emphasize-lines: 5
	
Commands Module
===============
	
.. autoclass:: rips.Commands
    :members:
    :undoc-members:
	
Example
-------
.. literalinclude:: ../../rips/PythonExamples/CommandExample.py
	:language: python
	:lines: 5-
	
Grid Module
===========

.. autoclass:: rips.Grid
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

.. autoclass:: rips.GridCaseGroup
    :members:


Project Module
==============

.. autoclass:: rips.Project
    :members:

Properties Module
=================

.. autoclass:: rips.Properties
    :members:

View Module
===========

.. autoclass:: rips.View
    :members:

Synchronous Example
-------------------
Read two properties, multiply them together and push the results back to ResInsight in a naïve way, by reading PORO into a list, then reading PERMX into a list, then multiplying them both in a resulting list and finally transferring back the list.

This is slow and inefficient, but works.

.. literalinclude:: ../../rips/PythonExamples/InputPropTestSync.py
	:language: python
	:lines: 5-
	
Asynchronous Example
--------------------
Read two properties at the same time chunk by chunk, multiply each chunk together and start transferring the result back to ResInsight as soon as the chunk is finished.

This is far more efficient.
	
.. literalinclude:: ../../rips/PythonExamples/InputPropTestAsync.py
	:language: python
	:lines: 5-
	
