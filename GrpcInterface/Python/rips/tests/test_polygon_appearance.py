"""
Tests for polygon appearance functionality.

These tests verify that polygon appearance properties can be read and modified
via the Python API.
"""

import os
import sys

import pytest

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))

import rips


@pytest.fixture
def project(rips_instance):
    """Get the current project."""
    return rips_instance.project


class TestPolygonAppearance:
    """Tests for PolygonAppearance functionality."""

    @pytest.fixture
    def polygon(self, project):
        """Create a test polygon."""
        polygon_collection = project.descendants(rips.PolygonCollection)[0]
        coordinates = [
            [100.0, 200.0, 300.0],
            [200.0, 200.0, 300.0],
            [200.0, 300.0, 300.0],
            [100.0, 300.0, 300.0],
        ]
        p = polygon_collection.create_polygon(
            name="Test Polygon", coordinates=coordinates
        )
        return p

    def test_access_appearance(self, polygon):
        """Test that appearance child object is accessible."""
        appearance = polygon.appearance()
        assert appearance is not None, "Appearance should be accessible"

    def test_default_values(self, polygon):
        """Test that default appearance values are correct."""
        appearance = polygon.appearance()
        assert appearance.is_closed is True
        assert appearance.show_lines is True
        assert appearance.show_spheres is False
        assert appearance.line_thickness == 3
        assert appearance.lock_polygon is False

    def test_set_line_color(self, polygon):
        """Test setting line color."""
        appearance = polygon.appearance()
        appearance.line_color = "#ff0000"
        appearance.update()

        # Re-read to verify persistence
        appearance2 = polygon.appearance()
        assert appearance2.line_color == "#ff0000"

    def test_set_sphere_color(self, polygon):
        """Test setting sphere color."""
        appearance = polygon.appearance()
        appearance.sphere_color = "#00ff00"
        appearance.update()

        appearance2 = polygon.appearance()
        assert appearance2.sphere_color == "#00ff00"

    def test_set_line_thickness(self, polygon):
        """Test setting line thickness."""
        appearance = polygon.appearance()
        appearance.line_thickness = 5
        appearance.update()

        appearance2 = polygon.appearance()
        assert appearance2.line_thickness == 5

    def test_set_show_spheres(self, polygon):
        """Test toggling sphere visibility."""
        appearance = polygon.appearance()
        appearance.show_spheres = True
        appearance.update()

        appearance2 = polygon.appearance()
        assert appearance2.show_spheres is True

    def test_set_sphere_radius_factor(self, polygon):
        """Test setting sphere radius factor."""
        appearance = polygon.appearance()
        appearance.sphere_radius_factor = 0.5
        appearance.update()

        appearance2 = polygon.appearance()
        assert abs(appearance2.sphere_radius_factor - 0.5) < 1e-6

    def test_set_is_closed(self, polygon):
        """Test setting closed polygon state."""
        appearance = polygon.appearance()
        appearance.is_closed = False
        appearance.update()

        appearance2 = polygon.appearance()
        assert appearance2.is_closed is False

    def test_set_lock_polygon(self, polygon):
        """Test setting lock polygon to plane."""
        appearance = polygon.appearance()
        appearance.lock_polygon = True
        appearance.polygon_plane_depth = 500.0
        appearance.update()

        appearance2 = polygon.appearance()
        assert appearance2.lock_polygon is True
        assert abs(appearance2.polygon_plane_depth - 500.0) < 1e-6
