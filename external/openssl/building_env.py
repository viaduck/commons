# creates a building environment for openssl
# - working directory
# - on windows: uses msys' bash for command execution (openssl's scripts need an UNIX-like environment with perl)

from subprocess import PIPE, Popen
from sys import argv
import os

env = os.environ
l = []

os_s = argv[1]                                      # operating system
offset = 2          # 0: this script's path, 1: operating system

if os_s == "WIN32":
    offset = 4  # 2: MSYS_BASH_PATH, 3: CMAKE_MAKE_PROGRAM

    #
    bash = argv[2]
    msys_path = os.path.dirname(bash)
    mingw_path = os.path.dirname(argv[3])

    # include path of msys binaries (perl, cd etc.) and building tools (gcc, ld etc.)
    env['PATH'] += ";".join([msys_path, mingw_path])

binary_openssl_dir_source = argv[offset]+"/"             # downloaded openssl source dir
l.extend(argv[offset+1:])                             # routed commands

if os_s == "WIN32"
    # we must emulate a UNIX environment to build openssl using mingw
    with Popen(bash, env=env, cwd=binary_openssl_dir_source, stdin=PIPE, universal_newlines=True) as proc:
        proc.stdin.write(" ".join(l))
else:
    Popen(" ".join(l), shell=True, env=env, cwd=binary_openssl_dir_source)
