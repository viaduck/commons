"""
Calls cogapp with required arguments for every enum file
"""

from subprocess import call
from sys import argv, executable
import os

ALLOWED_EXTENSIONS = ('.thx', '.the')

gen_type = argv[3]          # generation type. One of 'enum', 'protocol'
include_dir = argv[2]+"/include/"
definition_dir = os.getcwd() + "/" + gen_type + "/"

# create path if it doesn't exist already
if not os.path.exists(include_dir):
    os.makedirs(include_dir)

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
                 "-D base_path=" + definition_dir, "-o " + fullfname, "include/commons/" + gen_type + ".template.h"]
            if call(" ".join(l), shell=True) != 0:
                raise Exception("cog processing error on file: "+fname)
