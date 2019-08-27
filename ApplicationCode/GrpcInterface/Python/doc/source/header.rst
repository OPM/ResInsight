+++ 
title = "Python API - rips" 
published = true 
weight = 40 
+++ 


ResInsight has a `gRPC Remote Procedure Call <https://www.grpc.io/>`_ interface with a Python Client interface. This interface allows you to interact with a running ResInsight instance from a Python script.

The Python client package is available for install via the Python PIP package system with ``pip install rips`` as admin user, or ``pip install --user rips`` as a regular user.

On some systems the ``pip`` command may have to be replaced by ``python -m pip``.

In order for gRPC to be available, ResInsight needs to be built with the ``RESINSIGHT_ENABLE_GRPC`` option set. A valid gRPC build will show a message in the About dialog confirming gRPC is available:

.. image:: {{< relref "" >}}images/scripting/AboutGrpc.png

Furthermore, gRPC needs to be enabled in the Scripting tab of the Preference dialog:

.. image:: {{< relref "" >}}images/scripting/PrefGrpc.png
