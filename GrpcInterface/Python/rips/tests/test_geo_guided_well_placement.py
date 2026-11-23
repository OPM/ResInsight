import glob
import os
import sys

import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))

import dataroot
import rips

ENSEMBLE_GLOB = dataroot.PATH + "/Case_with_10_timesteps/Real*/*.EGRID"

# Cell centers (x, y) in the Brugge grid, and depths of the K layers 1-4 at the first point
TARGETS_XY = [
    (-6072.5, 15744.1),
    (-9725.9, 17654.3),
    (-13337.1, 19723.6),
]
TARGET_DEPTH = 6000.0


def _create_ensemble(rips_instance):
    file_names = sorted(glob.glob(ENSEMBLE_GLOB))
    assert len(file_names) > 1
    ensemble = rips_instance.project.create_reservoir_grid_ensemble(
        file_names=file_names, grouping_mode=rips.GroupingMode.None_
    )
    return ensemble, len(file_names)


def _create_source_well(rips_instance, name):
    well_path = rips_instance.project.well_path_collection().add_new_object(
        rips.ModeledWellPath
    )
    well_path.name = name
    well_path.update()

    geometry = well_path.well_path_geometry()
    for x, y in TARGETS_XY:
        geometry.append_well_target([x, y, TARGET_DEPTH], absolute=True)
    geometry.update()
    return well_path


def _target_depths(well_path):
    geometry = well_path.well_path_geometry()
    offset = geometry.reference_point[2]
    return [offset + t.target_point[2] for t in geometry.well_path_targets()]


def _generated_wells(rips_instance, source_name):
    return [
        w
        for w in rips_instance.project.well_paths()
        if w.name.startswith(source_name + "_")
    ]


def _add_k_filter(ensemble, name, start_k, cell_count_k):
    combined = ensemble.data_filter_collection().add_combined_filter(name=name)
    combined.add_range_filter(
        name=name + " range", start_k=start_k, cell_count_k=cell_count_k
    )
    return combined


def test_geo_guided_well_placement(rips_instance, initialize_test):
    ensemble, realization_count = _create_ensemble(rips_instance)
    zone = _add_k_filter(ensemble, "K3-4", start_k=3, cell_count_k=2)
    source = _create_source_well(rips_instance, "Source")

    placement = ensemble.add_geo_guided_well_placement(name="Placement")
    assert placement is not None
    assert placement.name == "Placement"
    assert len(ensemble.geo_guided_well_placements()) == 1

    placement.source_well_path = source
    placement.cell_filter = zone
    placement.perforation_filter = zone
    placement.fraction = 0.5
    placement.update()
    placement.compute()

    wells = _generated_wells(rips_instance, "Source")
    assert sorted(w.name for w in wells) == sorted(
        f"Source_R{i}" for i in range(realization_count)
    )

    for well in wells:
        depths = _target_depths(well)
        assert len(depths) == len(TARGETS_XY)
        # Layers K3-4 are located between ~5960 m and ~6110 m in the Brugge model
        for depth in depths:
            assert 5940.0 < depth < 6130.0
        assert any(abs(depth - TARGET_DEPTH) > 1.0 for depth in depths)

        perforations = well.completions().perforations().perforations()
        assert len(perforations) == 1
        assert perforations[0].end_measured_depth > perforations[0].start_measured_depth


def test_fraction_moves_targets(rips_instance, initialize_test):
    ensemble, _ = _create_ensemble(rips_instance)
    zone = _add_k_filter(ensemble, "K3-4", start_k=3, cell_count_k=2)
    source = _create_source_well(rips_instance, "Source")

    placement = ensemble.add_geo_guided_well_placement()
    placement.source_well_path = source
    placement.cell_filter = zone

    def depths_for_fraction(fraction):
        placement.fraction = fraction
        placement.update()
        placement.compute()
        well = rips_instance.project.well_path_by_name("Source_R0")
        return _target_depths(well)

    bottom = depths_for_fraction(0.0)
    top = depths_for_fraction(1.0)

    # Re-computing replaces the wells from the previous computation
    assert len(_generated_wells(rips_instance, "Source")) == len(
        ensemble.descendants(rips.EclipseCase)
    )
    for top_depth, bottom_depth in zip(top, bottom):
        assert top_depth < bottom_depth


def test_missing_input_fails(rips_instance, initialize_test):
    ensemble, _ = _create_ensemble(rips_instance)

    placement = ensemble.add_geo_guided_well_placement()
    placement.source_well_path = None
    placement.update()
    with pytest.raises(rips.RipsError):
        placement.compute()


def test_no_filter_hit_fails(rips_instance, initialize_test):
    ensemble, _ = _create_ensemble(rips_instance)
    empty = ensemble.data_filter_collection().add_combined_filter(name="Empty")
    empty.add_range_filter(name="Outside", start_k=100, cell_count_k=1)
    source = _create_source_well(rips_instance, "Source")

    placement = ensemble.add_geo_guided_well_placement()
    placement.source_well_path = source
    placement.cell_filter = empty
    placement.update()
    with pytest.raises(rips.RipsError):
        placement.compute()
    assert len(_generated_wells(rips_instance, "Source")) == 0
