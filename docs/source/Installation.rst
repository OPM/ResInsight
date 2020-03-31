Installation and Configuration
==============================

.. image:: images/python-logo-master-v3-TM.png

The ResInsight Python API is compatible with `Python 3 <https://www.python.org/download/releases/3.0/>`_. 
As admin user, the necessary Python client package is available for install via the Python PIP package system:

.. code-block:: console

   pip install rips

or as a regular user:
   
.. code-block:: console

   pip install --user rips
   
On some systems the `pip` command may have to be replaced by `python -m pip`.

To configure the **ResInsight Python Script Server**, check *Enable Python Script Server* and verify Python settings in the *Scripting* tab of the ResInsight *Preference* dialog.

.. image:: images/PrefGrpc.png

The availability of the ResInsight Python Script Server can be confirmed by ResInsight *About* dialog.
If unavailable, please consult ResInsight Build Instructions on `resinsight.org <https://resinsight.org/>`_.








