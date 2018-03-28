
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
Calls cogapp with required arguments for every enum file
"""

from subprocess import call
from sys import argv, executable
import os

ALLOWED_EXTENSIONS = ('.thx', '.the', '.sqx')

gen_type = argv[3]          # generation type. One of 'enum', 'protocol'
include_dir = argv[2]+"/include/"
definition_dir = os.getcwd() + "/" + gen_type + "/"

# create path for includes
os.makedirs(include_dir, exist_ok=True)

for root, dirs, files in os.walk(argv[1]):
    include_dir = argv[2]+"/include/"
    for file in files:
        fname, ext = os.path.splitext(file)
        if ext in ALLOWED_EXTENSIONS:
            fname = os.path.join(root, fname)
            fname = os.path.relpath(fname, argv[1])
            fullfname = include_dir+fname
            fullfname += ".h"
            fname += ext

            if not os.path.exists(os.path.dirname(fullfname)):
                os.makedirs(os.path.dirname(fullfname))
            l = ['"' + executable +'"', "-m cogapp", "-d", "-I tools/", "-D filename=" + fname,
                 "-D base_path=" + definition_dir, "-o " + fullfname, "tools/generators/" + gen_type + ".template.h"]
            if call(" ".join(l), shell=True) != 0:
                raise Exception("cog processing error on file: "+fname)
