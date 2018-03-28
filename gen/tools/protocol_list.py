
# Copyright (C) 2015-2018 The ViaDuck Project
#
# This file is part of Commons.
#
# Commons is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Commons is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Commons.  If not, see <http://www.gnu.org/licenses/>.

"""
Prints list of generation definition files (relative path)
"""

from sys import argv
import os

outp = ""
include_dir = os.getcwd()+"/include/"
for root, dirs, files in os.walk(argv[1]):
    for file in files:
        fname, ext = os.path.splitext(file)
        if ext in ('.thx', '.the', '.sqx'):
            fname = os.path.join(root, fname)
            fname = os.path.relpath(fname, argv[1])

            fname = include_dir+fname
            outp += fname+".h;"

print(outp[:-1], end="")
