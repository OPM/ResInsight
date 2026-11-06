import inspect
import uuid
from collections.abc import Iterable

from .exception import RipsError
from .pdmobject import add_method
from .project import Project
from .resinsight_classes import (
    EnsembleSurface,
    RegularSurface,
    SurfaceCollection,
    SurfaceInterface,
    TriangulatedSurface,
)
from typing import List, Optional, Sequence, Type, TypeVar, Union

SurfaceT = TypeVar("SurfaceT", bound=SurfaceInterface)


@add_method(RegularSurface)
def set_property(self: RegularSurface, name: str, values: List[float]) -> None:
    """Sets a property on a regular surface.

    Arguments:
        name(str): Name of the property.
        values (List[float]): Values to set (float32).
          Should be of nx*ny size (see RegularSurface).
    """

    key = "{}_{}".format(uuid.uuid4(), "regular_surface_key")

    project = self.ancestor(Project)
    if project:
        project.set_key_values(key, values)
        self.set_property_from_key(name=name, value_key=key)


@add_method(RegularSurface)
def available_properties(self: RegularSurface) -> List[str]:
    """Gets the list of available property names on a regular surface.

    Returns:
        List[str]: Names of all properties set on this surface.
    """
    result = self.property_names()
    return result.values


@add_method(RegularSurface)
def get_property(self: RegularSurface, name: str) -> List[float]:
    """Gets a property from a regular surface.

    Arguments:
        name(str): Name of the property.

    Returns:
        List[float]: Values of the property (float32).
          Size is nx*ny (see RegularSurface).
    """
    key = "{}_{}".format(uuid.uuid4(), "regular_surface_key")

    project = self.ancestor(Project)
    if not project:
        raise RipsError("Could not find parent project")

    try:
        self.get_property_to_key(name=name, value_key=key)
        return project.key_values(key)
    finally:
        try:
            project.remove_key_values(key)
        except Exception:
            pass


def _flatten(values: Sequence[Union[float, Iterable[float]]]) -> List[float]:
    flat: List[float] = []
    for value in values:
        if isinstance(value, Iterable):
            flat.extend(float(v) for v in value)
        else:
            flat.append(float(value))
    return flat


@add_method(TriangulatedSurface)
def set_geometry(
    self: TriangulatedSurface,
    vertices: Sequence[Union[float, Iterable[float]]],
    triangles: Sequence[Union[int, Iterable[int]]],
) -> None:
    """Sets the vertices and triangles of a triangulated surface.

    Properties already set on the surface are removed.

    Arguments:
        vertices (List[List[float]]): x, y and depth of each vertex, depth positive
          downwards (float32). A flat list of x, y, depth values is also accepted.
        triangles (List[List[int]]): Three vertex indices per triangle, zero based.
          A flat list of indices is also accepted.
    """
    project = self.ancestor(Project)
    if not project:
        raise RipsError("Could not find parent project")

    vertices_key = "{}_{}".format(uuid.uuid4(), "triangulated_surface_vertices")
    triangles_key = "{}_{}".format(uuid.uuid4(), "triangulated_surface_triangles")

    project.set_key_values(vertices_key, _flatten(vertices))
    project.set_key_values(triangles_key, _flatten(triangles))
    self.set_geometry_from_keys(vertices_key=vertices_key, triangles_key=triangles_key)


@add_method(TriangulatedSurface)  # type: ignore[no-redef]
def set_property(  # noqa: F811
    self: TriangulatedSurface, name: str, values: List[float]
) -> None:
    """Sets a property on a triangulated surface.

    Arguments:
        name(str): Name of the property.
        values (List[float]): One value per vertex (float32).
    """
    project = self.ancestor(Project)
    if not project:
        raise RipsError("Could not find parent project")

    key = "{}_{}".format(uuid.uuid4(), "triangulated_surface_key")
    project.set_key_values(key, values)
    self.set_property_from_key(name=name, value_key=key)


@add_method(TriangulatedSurface)  # type: ignore[no-redef]
def available_properties(self: TriangulatedSurface) -> List[str]:  # noqa: F811
    """Gets the list of available property names on a triangulated surface.

    Returns:
        List[str]: Names of all properties set on this surface.
    """
    result = self.property_names()
    return result.values


@add_method(TriangulatedSurface)  # type: ignore[no-redef]
def get_property(self: TriangulatedSurface, name: str) -> List[float]:  # noqa: F811
    """Gets a property from a triangulated surface.

    Arguments:
        name(str): Name of the property.

    Returns:
        List[float]: Values of the property (float32), one value per vertex.
    """
    key = "{}_{}".format(uuid.uuid4(), "triangulated_surface_key")

    project = self.ancestor(Project)
    if not project:
        raise RipsError("Could not find parent project")

    try:
        self.get_property_to_key(name=name, value_key=key)
        return project.key_values(key)
    finally:
        try:
            project.remove_key_values(key)
        except Exception:
            pass


def _check_surface_type(surface_type: Type[SurfaceT]) -> None:
    if not (
        inspect.isclass(surface_type) and issubclass(surface_type, SurfaceInterface)
    ):
        raise RipsError(
            "surface_type must be a surface class such as rips.SurfaceInterface, "
            "rips.Surface or rips.RegularSurface, got {}".format(surface_type)
        )


@add_method(SurfaceCollection)
def surfaces(
    self: SurfaceCollection,
    surface_type: Type[SurfaceT] = SurfaceInterface,
    recursive: bool = False,
    include_ensembles: bool = False,
) -> List[SurfaceT]:
    """Get the surfaces in this folder

    Arguments:
        surface_type (class): Only return surfaces of this class, subclasses included.
            rips.SurfaceInterface (the default) returns every surface, rips.Surface
            returns file surfaces only, and rips.RegularSurface also returns
            rips.RegularFileSurface.
        recursive (bool): Also return the surfaces in sub folders.
        include_ensembles (bool): When recursive, also descend into ensemble surfaces
            (rips.EnsembleSurface), which hold six statistics surfaces and one surface
            per realization. Off by default, since that can be a long list.

    Returns:
        List of surfaces of the requested class, this folder first, then its sub folders
    """
    _check_surface_type(surface_type)

    result: List[SurfaceT] = [
        s for s in self.surfaces_field() if isinstance(s, surface_type)
    ]
    if recursive:
        for folder in self.sub_collections():
            if isinstance(folder, EnsembleSurface) and not include_ensembles:
                continue
            result.extend(
                folder.surfaces(
                    surface_type, recursive=True, include_ensembles=include_ensembles
                )
            )
    return result


@add_method(Project)  # type: ignore[no-redef]
def surfaces(  # noqa: F811
    self: Project,
    surface_type: Type[SurfaceT] = SurfaceInterface,
    include_ensembles: bool = False,
) -> List[SurfaceT]:
    """Get all surfaces in the project, from the top surface folder and its sub folders

    Arguments:
        surface_type (class): Only return surfaces of this class, subclasses included.
            rips.SurfaceInterface (the default) returns every surface, rips.Surface
            returns file surfaces only, and rips.RegularSurface also returns
            rips.RegularFileSurface.
        include_ensembles (bool): Also return the statistics and realization surfaces
            of ensemble surfaces (rips.EnsembleSurface). Off by default.

    Returns:
        List of surfaces of the requested class
    """
    folder = self.surface_folder()
    if folder is None:
        return []
    return folder.surfaces(
        surface_type, recursive=True, include_ensembles=include_ensembles
    )


@add_method(Project)
def surface_by_name(
    self: Project,
    name: str,
    surface_type: Type[SurfaceT] = SurfaceInterface,  # type: ignore[assignment]
    folder: Optional[SurfaceCollection] = None,
) -> Optional[SurfaceT]:
    """Get a surface by name

    Names are only unique within one folder. Without a folder the first match is
    returned, searching the top surface folder first and then its sub folders. In that
    case ensemble surfaces are excluded. The name is matched against
    surface_user_description, so a depth offset shown in the tree does not affect the
    match.

    Arguments:
        name (str): The surface name (surface_user_description)
        surface_type (class): Only consider surfaces of this class, subclasses included
        folder (SurfaceCollection): Only look in this folder, sub folders excluded. Any
            folder can be passed, including a rips.EnsembleSurface. Note that
            project.surface_folder(name) only finds direct sub folders of the top
            folder; use folder.sub_collections() to reach nested folders.

    Returns:
        The surface, or None when no surface has that name
    """
    if folder is not None:
        candidates = folder.surfaces(surface_type)
    else:
        candidates = self.surfaces(surface_type)
    for surface in candidates:
        if surface.surface_user_description == name:
            return surface
    return None
