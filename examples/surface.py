#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

from hilma import Mesh, Polyline, surface, savePly
import numpy as np

polylineA = Polyline( [[100, 0], [100, 100], [0, 100], [0, 0]] )
polylineB = Polyline( np.array([[75, 25], [75, 75], [25, 75], [25, 25]], dtype=np.float32) )

mesh = surface( polylineA )

savePly("surface.ply", mesh, False)