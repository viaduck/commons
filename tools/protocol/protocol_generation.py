"""
Calls cogapp with required arguments for every protocol file
"""

from subprocess import call
from sys import argv, executable
import os

protocol_dir = os.getcwd()+"/protocol/"
if not os.path.exists(argv[2]+"/include/"):
    os.makedirs(argv[2]+"/include/")
include_dir = argv[2]+"/include/"
for root, dirs, files in os.walk(argv[1]):
    for file in files:
        fname, ext = os.path.splitext(file)
        if ext == '.thx':
            fname = os.path.join(root, fname)
            fname = os.path.relpath(fname, argv[1])
            fullfname = include_dir+fname
            fullfname += ".h"
            fname += ".thx"

            if not os.path.exists(os.path.dirname(fullfname)):
                os.makedirs(os.path.dirname(fullfname))
            l = ['"'+executable+'"', "-m cogapp", "-d", "-I tools/protocol/", "-D filename="+fname, "-D base_path="+protocol_dir, "-o "+fullfname, "include/libCom/message.template.h"]
            if call(" ".join(l), shell=True) != 0:
                raise Exception("cog processing error on file: "+fname)
