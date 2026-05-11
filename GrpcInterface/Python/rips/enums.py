"""Typed enums mirroring the proto enums used by the rips.case API.

These are hand-written counterparts to proto enums defined in ``Properties.proto``
and ``NNCProperties.proto``. They give users typed, autocompletable enum members
while remaining wire-compatible with the existing string-based API (``StrEnum``
members are strings).

Auto-generated ``StrEnum`` classes for ``caf::AppEnum`` scriptable fields live in
``rips.generated.generated_classes`` and are emitted by ``cafPdmPythonGenerator``.
``PorosityModelType`` is one of those auto-generated enums and is imported from
``resinsight_classes``; the enums in this module cover the gRPC-only proto enums
that the auto-generator does not see.
"""

from enum import StrEnum


# Keep in sync with PropertyType in GrpcInterface/GrpcProtos/Properties.proto
class PropertyType(StrEnum):
    DYNAMIC_NATIVE = "DYNAMIC_NATIVE"
    STATIC_NATIVE = "STATIC_NATIVE"
    SOURSIMRL = "SOURSIMRL"
    GENERATED = "GENERATED"
    INPUT_PROPERTY = "INPUT_PROPERTY"
    FORMATION_NAMES = "FORMATION_NAMES"
    FLOW_DIAGNOSTICS = "FLOW_DIAGNOSTICS"
    INJECTION_FLOODING = "INJECTION_FLOODING"
    REMOVED = "REMOVED"
    UNDEFINED = "UNDEFINED"


# Keep in sync with PropertyDataType in GrpcInterface/GrpcProtos/Properties.proto
class PropertyDataType(StrEnum):
    FLOAT = "FLOAT"
    INTEGER = "INTEGER"


# Keep in sync with NNCPropertyType in GrpcInterface/GrpcProtos/NNCProperties.proto
class NNCPropertyType(StrEnum):
    NNC_DYNAMIC = "NNC_DYNAMIC"
    NNC_STATIC = "NNC_STATIC"
    NNC_GENERATED = "NNC_GENERATED"
