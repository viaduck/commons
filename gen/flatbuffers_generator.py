
# Copyright (C) 2018 The ViaDuck Project
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

import subprocess
import os
from sys import argv

generators = ['flatbuffers']
allowed_exts = ['.fbs']

rel_definitions = "{generator}/"


def generate(infile, flatc, gendir, outdir):
    # assemble shell arguments
    shell_args = [
        # quote to avoid platform issues
        '"' + flatc + '"',
        # generate c++
        "--cpp",
        # include directory (of definition files)
        "-I " + gendir,
        # output the cpp files
        "-o " + outdir,
        # fbs definition file
        infile]

    # call generator on shell to generate an instance of the template
    subprocess.call(" ".join(shell_args), shell=True)


def list_files(gen_dir, out_dir):
    """
    Collects all definition files from gendir and outputs the filenames in outdir they will have when generated.

    :return List of dicts with src, template and out keys
    """
    result = []

    for generator in generators:
        # this generators relative paths
        gen_rel = rel_definitions.format(generator=generator)

        # definitions of this generator
        gen_def_dir = os.path.join(gen_dir, gen_rel)
        # outputs of this generator
        gen_out_dir = os.path.join(out_dir, gen_rel)

        # walk all definitions
        for dir_path, _, files in os.walk(gen_def_dir):
            for file in files:
                def_file = os.path.join(dir_path, file)
                def_file_base, ext = os.path.splitext(def_file)
                if ext in allowed_exts:
                    # make def path relative to def dir
                    def_rel_base = os.path.relpath(def_file_base, gen_def_dir)

                    # transfer it to out dir
                    out_file_h = os.path.join(gen_out_dir, def_rel_base) + "_generated.h"
                    
                    # out dir for this file
                    out_dir_h = os.path.dirname(out_file_h)

                    # add to result
                    result.append({'src': def_file, 'h_out': out_file_h, 'dest': out_dir_h})

    return result


if __name__ == "__main__":
    # at least a verb and two paths must be supplied
    if len(argv) < 5:
        raise Exception("Invalid arguments")

    # extract args
    _verb, _gen_dir, _out_dir, _flatc = argv[1], argv[2], argv[3], argv[4]

    # all source files, their templates and respective output files
    file_list = list_files(_gen_dir, _out_dir)

    if _verb == "list":
        # only list the output files, don't actually do anything
        # also, use CMake list format "a;b;c"
        print(";".join(f['h_out'] for f in file_list), end="")

    elif _verb == "depend":
        # build a list of all the dependencies of the output files
        # also, use CMake list format "a;b;c"
        print(";".join(f['src'] for f in file_list), end="")

    elif _verb == "generate":
        # actually generate the files
        for f in file_list:
            generate(f['src'], _flatc, _gen_dir, f['dest'])

    else:
        raise Exception("Unknown verb")
