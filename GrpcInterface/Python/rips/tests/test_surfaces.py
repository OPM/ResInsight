import sys
import os
import tempfile
from pathlib import Path
import pytest

sys.path.insert(1, os.path.join(sys.path[0], "../../"))
import rips

import dataroot


def test_create_and_export_surface(rips_instance, initialize_test):
    case_path = dataroot.PATH + "/Case_with_10_timesteps/Real0/BRUGGE_0000.EGRID"
    case = rips_instance.project.load_case(path=case_path)
    assert len(case.grids()) == 1

    surface_collection = rips_instance.project.descendants(rips.SurfaceCollection)[0]

    surface = surface_collection.new_surface(case, 5)
    assert surface

    with tempfile.TemporaryDirectory(prefix="rips") as tmpdirname:
        path = Path(tmpdirname, "mysurface.ts")
        print("Temporary folder: ", path.as_posix())

        fname = surface.export_to_file(path.as_posix())
        assert len(fname.values) == 1

        assert path.exists()
