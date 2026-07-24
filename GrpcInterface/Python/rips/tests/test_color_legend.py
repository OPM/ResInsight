import os
import sys

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import dataroot
import rips


def test_color_legend_collection_accessor(rips_instance, initialize_test):
    """The project exposes the ColorLegendCollection via a dedicated method."""
    collection = rips_instance.project.color_legend_collection()
    assert collection is not None
    assert isinstance(collection, rips.ColorLegendCollection)


def test_create_color_legend_and_items(rips_instance, initialize_test):
    collection = rips_instance.project.color_legend_collection()

    legend = collection.create_color_legend(name="My Legend")
    assert legend is not None
    assert legend.color_legend_name == "My Legend"

    red = legend.add_color_legend_item(
        category_value=1, category_name="one", color="#ff0000"
    )
    green = legend.add_color_legend_item(
        category_value=2, category_name="two", color="#00ff00"
    )
    blue = legend.add_color_legend_item(
        category_value=3, category_name="three", color="#0000ff"
    )

    assert red.category_value == 1
    assert red.category_name == "one"
    assert red.color == "#ff0000"

    assert green.category_value == 2
    assert green.category_name == "two"
    assert green.color == "#00ff00"

    assert blue.category_value == 3
    assert blue.category_name == "three"
    assert blue.color == "#0000ff"


def test_color_legend_item_default_color(rips_instance, initialize_test):
    """When created via the command, an item reports the color it was given."""
    collection = rips_instance.project.color_legend_collection()
    legend = collection.create_color_legend(name="Named Color")

    item = legend.add_color_legend_item(
        category_value=0, category_name="orange", color="orange"
    )
    # QColor normalizes named colors to their hex representation.
    assert item.color == "#ffa500"


def test_set_and_delete_default_color_legend_for_result(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert case is not None

    collection = rips_instance.project.color_legend_collection()
    legend = collection.create_color_legend(name="Result Legend")
    legend.add_color_legend_item(category_value=0, category_name="a", color="#112233")
    legend.add_color_legend_item(category_value=1, category_name="b", color="#445566")

    # Should not raise for a known result name.
    collection.set_default_color_legend_for_result(
        case=case, result_name="SOIL", color_legend=legend
    )

    # Deleting the binding should also succeed.
    collection.delete_color_legend(case=case, result_name="SOIL")


def test_discrete_property_category_round_trip(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert case is not None

    # Before anything is set, the getters return empty dicts.
    assert case.discrete_property_category_names("FACIES") == {}
    assert case.discrete_property_category_colors("FACIES") == {}

    expected_names = {0: "Sand", 1: "Shale", 2: "Coal"}
    expected_colors = {0: "#e6c878", 1: "#646464", 2: "#202020"}

    case.set_discrete_property_category_names(
        property_name="FACIES",
        value_names=expected_names,
        value_colors=expected_colors,
    )

    assert case.discrete_property_category_names("FACIES") == expected_names
    assert case.discrete_property_category_colors("FACIES") == expected_colors

    # Clearing the mapping with an empty dict should make the getters
    # report empty dicts again.
    case.set_discrete_property_category_names(property_name="FACIES", value_names={})
    assert case.discrete_property_category_names("FACIES") == {}
    assert case.discrete_property_category_colors("FACIES") == {}


def test_discrete_property_category_no_duplicate_legend(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert case is not None

    collection = rips_instance.project.color_legend_collection()

    def legend_count():
        return len(collection.descendants(rips.ColorLegend))

    case.set_discrete_property_category_names(
        property_name="FACIES", value_names={0: "Sand", 1: "Shale"}
    )
    after_first = legend_count()

    # Calling again for the same property must replace, not accumulate.
    case.set_discrete_property_category_names(
        property_name="FACIES", value_names={0: "Sand", 1: "Shale", 2: "Coal"}
    )
    assert legend_count() == after_first

    # The mapping still reflects the latest call.
    assert case.discrete_property_category_names("FACIES") == {
        0: "Sand",
        1: "Shale",
        2: "Coal",
    }


def test_discrete_property_category_update_preserves_legend_identity(
    rips_instance, initialize_test
):
    case_path = dataroot.PATH + "/TEST10K_FLT_LGR_NNC/TEST10K_FLT_LGR_NNC.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert case is not None

    collection = rips_instance.project.color_legend_collection()

    def legend_count():
        return len(collection.descendants(rips.ColorLegend))

    legend1 = case.set_discrete_property_category_names(
        property_name="FACIES", value_names={0: "Sand", 1: "Shale"}
    )
    assert legend1 is not None
    after_first = legend_count()

    # A repeated call must update the existing legend object in place so that
    # views referencing it keep their binding.
    new_names = {0: "Sandstone", 1: "Shale", 2: "Coal"}
    new_colors = {0: "#e6c878", 1: "#646464", 2: "#202020"}
    legend2 = case.set_discrete_property_category_names(
        property_name="FACIES", value_names=new_names, value_colors=new_colors
    )
    assert legend2.address() == legend1.address()
    assert legend_count() == after_first

    # The items are fully replaced, with no leftovers from the first call.
    assert case.discrete_property_category_names("FACIES") == new_names
    assert case.discrete_property_category_colors("FACIES") == new_colors

    # The legend can be renamed in place as well.
    legend3 = case.set_discrete_property_category_names(
        property_name="FACIES", value_names=new_names, legend_name="My Facies"
    )
    assert legend3.address() == legend1.address()
    assert legend3.color_legend_name == "My Facies"
