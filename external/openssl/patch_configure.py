# handles working directory changes and calls openssl's internal config script since we can't specify
# current working directory in CMake's ExternalProject_Add(..)

from subprocess import call
from sys import argv
import os

os_s = argv[1]
binary_openssl_dir_source = argv[2]+"/"

os.chdir(binary_openssl_dir_source)

l = [binary_openssl_dir_source]
l[0] += "config"

##
print(l)
l.extend(argv[3:])
print(l)
call(" ".join(l), shell=True)