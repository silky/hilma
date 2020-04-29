#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

from hilma import Mesh, PlyOps

size = 1024

mesh = Mesh()

for y in range(size):
    for x in range(size):
        u = x/float(size-1);
        v = y/float(size-1);
        mesh.addVertex(u, v, 1.);

PlyOps.save("plane.ply", mesh, False)