#  Copyright (C) 2015  Equinor ASA, Norway.
#
#  The file 'well_trajectory.py' is part of ERT - Ensemble based Reservoir Tool.
#
#  ERT is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE.
#
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
#  for more details.

import sys
from os.path import isfile
from collections import namedtuple

TrajectoryPoint = namedtuple('TrajectoryPoint',
                             ['utm_x', 'utm_y',
                              'measured_depth', 'true_vertical_depth',
                              'zone'])

def _read_point(line):
    line = line.partition("--")[0]
    line = line.strip()
    if not line:
        return None

    point = line.split()
    if len(point) not in (4, 5):
        fmt = 'utm_x utm_y md tvd [zone]'
        err = 'Trajectory data file not on correct format: "%s".'
        err += '  zone is optional.'
        raise UserWarning(err % fmt)
    return point

def _parse_point(point):
    try:
        utm_x = float(point[0])
        utm_y = float(point[1])
        md = float(point[2])
        tvd = float(point[3])
    except ValueError:
        raise UserWarning("Error: Failed to extract data from line %s\n" % str(point))
    zone = None
    if len(point) > 4:
        zone = point[4]
    return TrajectoryPoint(utm_x, utm_y, md, tvd, zone)


class WellTrajectory:

    def __init__(self, filename):
        self._points = []
        if not isfile(filename):
            raise IOError('No such file "%s"' % filename)

        with open(filename) as fileH:
            for line in fileH.readlines():
                point = _read_point(line)
                if not point:
                    continue
                traj = _parse_point(point)
                self._points.append(traj)

    def __len__(self):
        return len(self._points)


    def __getitem__(self, index):
        if index < 0:
            index += len(self)

        return self._points[index]


    def __repr__(self):
        return 'WellTrajectory(len=%d)' % len(self)

    def __str__(self):
        return 'WellTrajectory(%s)' % str(self._points)
