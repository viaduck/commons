from subprocess import call
from sys import argv
import os

#outp = ""
protocol_dir = os.getcwd()+"/protocol/"
if not os.path.exists(argv[2]+"/include/"):
    os.makedirs(argv[2]+"/include/")
with open(argv[1], "r") as f:
    for line in f:
        if line[:1] == "#":
            continue
        line = line.strip()
        l = ["cog.py", "-d", "-I tools/", "-D filename="+line.replace(".h", ".thx"), "-D base_path="+protocol_dir, "-o "+argv[2]+"/include/"+line, "include/message.template.h"]
        call(" ".join(l), shell=True)

#print(outp, end="")