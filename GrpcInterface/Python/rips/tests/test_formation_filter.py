import os
import shutil
import sys

import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot

# K ranges are one based. 'Upper' occurs twice: a formation split over two K ranges.
SINGLE_CASE_LYR = """'Upper'   1 - 10
'Middle' 11 - 20
'Upper'  21 - 25
'Lower'  26 - 36
"""

# Tracked grids used as ensemble realizations
SOURCE_GRIDS = {
    1: dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000",
    2: dataroot.PATH + "/Case_with_10_timesteps/Real10/BRUGGE_0010",
    3: dataroot.PATH + "/Case_with_10_timesteps/Real30/BRUGGE_0030",
}


def _write(path, text):
    with open(path, "w") as f:
        f.write(text)
    return str(path)


def _open_case_with_formations(rips_instance, tmp_path):
    case = rips_instance.project.load_case(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    )
    lyr = _write(tmp_path / "formations.lyr", SINGLE_CASE_LYR)
    rips_instance.project.import_formation_names(formation_files=[lyr])
    return case


def _ensemble(rips_instance, tmp_path, lyr_per_realization):
    """Grid ensemble with a formation file for each realization.

    The grid files are placed in realization-N/iter-1/eclipse/model, so the
    formation file is picked up from realization-N/iter-1.
    """
    file_names = []
    for realization, lyr in lyr_per_realization.items():
        iteration = tmp_path / "realization-{}".format(realization) / "iter-1"
        model = iteration / "eclipse" / "model"
        model.mkdir(parents=True)
        for extension in ["EGRID", "INIT"]:
            shutil.copy(
                "{}.{}".format(SOURCE_GRIDS[realization], extension),
                model / "CASE_R{}.{}".format(realization, extension),
            )
        _write(iteration / "formations.lyr", lyr)
        file_names.append(str(model / "CASE_R{}.EGRID".format(realization)))

    return rips_instance.project.create_reservoir_grid_ensemble(file_names=file_names)


def _realizations_by_number(ensemble):
    realizations = {}
    for case in ensemble.descendants(rips.EclipseCase):
        for number in SOURCE_GRIDS:
            if "CASE_R{}".format(number) in case.name:
                realizations[number] = case
    return realizations


def _selected_k_layers(case, cell_filter):
    """One based K layers with cells selected by the filter in the given case"""
    mask = case.filtered_cells(cell_filter)
    dimensions = case.grid(0).dimensions()
    cells_per_layer = dimensions.i * dimensions.j
    return {
        cell_index // cells_per_layer + 1
        for cell_index, selected in enumerate(mask)
        if selected
    }


def test_add_formation_filter_to_data_filter_collection(
    rips_instance, initialize_test, tmp_path
):
    case = _open_case_with_formations(rips_instance, tmp_path)
    data_filters = case.data_filter_collection()

    formation = data_filters.add_formation_filter(formation_names=["Middle"])
    assert isinstance(formation, rips.FormationFilter)
    assert len(data_filters.filters()) == 1
    assert formation.selected_formation_names() == ["Middle"]
    assert "Middle" in formation.name

    # Empty selects all, and duplicates are reported once
    all_formations = data_filters.add_formation_filter()
    assert all_formations.selected_formation_names() == ["Upper", "Middle", "Lower"]

    # 'Upper' is selected in both of its K ranges, and named once
    upper = data_filters.add_formation_filter(formation_names=["Upper", "Lower"])
    assert upper.selected_formation_names() == ["Upper", "Lower"]
    assert upper.name.count("Upper") == 1


def test_add_formation_filter_to_combined_filter(
    rips_instance, initialize_test, tmp_path
):
    case = _open_case_with_formations(rips_instance, tmp_path)
    combined = case.data_filter_collection().add_combined_filter(combine_mode="AND")

    formation = combined.add_formation_filter(
        formation_names=["Lower", "Upper"], filter_mode="EXCLUDE"
    )
    assert isinstance(formation, rips.FormationFilter)
    assert len(combined.filters()) == 1
    assert "excl." in formation.name
    assert formation.selected_formation_names() == ["Upper", "Lower"]

    formation.set_selected_formation_names(formation_names=["Middle"])
    assert formation.selected_formation_names() == ["Middle"]


def test_formation_filter_selects_cells_by_formation(
    rips_instance, initialize_test, tmp_path
):
    case = _open_case_with_formations(rips_instance, tmp_path)
    data_filters = case.data_filter_collection()

    include = data_filters.add_formation_filter(formation_names=["Upper"])
    exclude = data_filters.add_formation_filter(
        formation_names=["Upper"], filter_mode="EXCLUDE"
    )

    include_mask = case.filtered_cells(include)
    exclude_mask = case.filtered_cells(exclude)

    assert len(include_mask) == len(exclude_mask)
    assert 0 < sum(include_mask) < len(include_mask)
    assert all(a != b for a, b in zip(include_mask, exclude_mask))

    # 'Upper' covers both of its K ranges. The mask holds all cells of the main grid.
    dimensions = case.grid(0).dimensions()
    cells_per_layer = dimensions.i * dimensions.j
    assert len(include_mask) == cells_per_layer * dimensions.k
    for cell_index, selected in enumerate(include_mask):
        k = cell_index // cells_per_layer + 1
        assert bool(selected) == (1 <= k <= 10 or 21 <= k <= 25)


def test_formation_filter_errors(rips_instance, initialize_test, tmp_path):
    case = _open_case_with_formations(rips_instance, tmp_path)
    data_filters = case.data_filter_collection()

    with pytest.raises(rips.RipsError, match="Does Not Exist"):
        data_filters.add_formation_filter(formation_names=["Does Not Exist"])

    with pytest.raises(rips.RipsError, match="filter mode"):
        data_filters.add_formation_filter(filter_mode="SOMETIMES")

    formation = data_filters.add_formation_filter(formation_names=["Middle"])
    with pytest.raises(rips.RipsError):
        formation.set_selected_formation_names(formation_names=["Does Not Exist"])
    assert formation.selected_formation_names() == ["Middle"]

    # No failed attempt leaves a filter behind
    assert len(data_filters.filters()) == 1


def test_formation_filter_without_formations_is_error(rips_instance, initialize_test):
    case = rips_instance.project.load_case(
        dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    )
    data_filters = case.data_filter_collection()

    with pytest.raises(rips.RipsError, match="No formation names"):
        data_filters.add_formation_filter(formation_names=["Upper"])
    assert len(data_filters.filters()) == 0


def test_ensemble_formation_filter_same_formations(
    rips_instance, initialize_test, tmp_path
):
    lyr = "'Overburden' 1 - 2\n'Reservoir' 3 - 5\n'Underburden' 6 - 9\n"
    ensemble = _ensemble(rips_instance, tmp_path, {r: lyr for r in SOURCE_GRIDS})

    expected = ["Overburden", "Reservoir", "Underburden"]
    assert ensemble.formation_names() == expected
    assert ensemble.formation_names(union=True) == expected

    combined = ensemble.data_filter_collection().add_combined_filter(name="Reservoir")
    formation = combined.add_formation_filter(formation_names=["Reservoir"])
    assert formation.selected_formation_names() == ["Reservoir"]

    formation = ensemble.data_filter_collection().add_formation_filter(
        formation_names=["Overburden"]
    )
    assert formation.selected_formation_names() == ["Overburden"]


def test_ensemble_formation_filter_different_k_ranges_match(
    rips_instance, initialize_test, tmp_path
):
    ensemble = _ensemble(
        rips_instance,
        tmp_path,
        {
            1: "'Top' 1 - 3\n'Base' 4 - 9\n",
            2: "'Top' 1 - 5\n'Base' 6 - 9\n",
        },
    )

    formation = ensemble.data_filter_collection().add_formation_filter(
        formation_names=["Base"]
    )
    assert formation.selected_formation_names() == ["Base"]

    # Each realization selects the K range of 'Base' in its own formation file
    realizations = _realizations_by_number(ensemble)
    assert _selected_k_layers(realizations[1], formation) == {4, 5, 6, 7, 8, 9}
    assert _selected_k_layers(realizations[2], formation) == {6, 7, 8, 9}


def test_ensemble_formation_filter_resolves_names_per_realization(
    rips_instance, initialize_test, tmp_path
):
    ensemble = _ensemble(
        rips_instance,
        tmp_path,
        {
            1: "'Top' 1 - 3\n'Base' 4 - 9\n",
            2: "'Top' 1 - 3\n'Base' 4 - 9\n",
            3: "'Base' 1 - 3\n'Top' 4 - 6\n'Extra' 7 - 9\n",
        },
    )

    assert ensemble.formation_names() == ["Top", "Base"]
    assert ensemble.formation_names(union=True) == ["Top", "Base", "Extra"]

    # The selection is stored against the main case, the first realization
    formation = ensemble.data_filter_collection().add_formation_filter(
        formation_names=["Top"]
    )
    assert formation.selected_formation_names() == ["Top"]

    # R3 has the formations in another order, and selects its own 'Top' cells
    realizations = _realizations_by_number(ensemble)
    assert _selected_k_layers(realizations[1], formation) == {1, 2, 3}
    assert _selected_k_layers(realizations[3], formation) == {4, 5, 6}

    # A name only found in other realizations than the main case
    with pytest.raises(rips.RipsError, match="exists in realizations"):
        ensemble.data_filter_collection().add_formation_filter(
            formation_names=["Extra"]
        )
