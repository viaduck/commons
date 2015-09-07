from subprocess import call
from sys import argv
import os

outp = ""
include_dir = os.getcwd()+"/include/"
with open(argv[1], "r") as f:
    for line in f:
        if line[:1] == "#":
            continue
        line = line.strip()
        outp += include_dir+line+";"

print(outp[:-1], end="")