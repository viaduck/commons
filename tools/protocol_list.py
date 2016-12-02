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
        if ext in ('.thx', '.the'):
            fname = os.path.join(root, fname)
            fname = os.path.relpath(fname, argv[1])

            fname = include_dir+fname
            outp += fname+".h;"

print(outp[:-1], end="")
