import os
import sys

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


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
