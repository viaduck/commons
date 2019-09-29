
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
from sys import executable, argv

generators = ['enum', 'flatbuffers', 'sqx', 'bit']
allowed_exts = ['.the', '.thx', '.sqx', '.btx']

rel_definitions = "{generator}/"
rel_template = "generators/{generator}.template.h"
rel_generator = "generators/gen_{generator}.py"


def generate(def_base_dir, infile, template, outfile):
    # assemble cogapp arguments
    shell_args = [
        # quote to avoid platform issues
        '"' + executable + '"',
        # run cogapp module
        "-m cogapp",
        # delete [[[cog]]] lines from outfile
        "-d",
        # pass "def_base_dir" global variable to cog script
        "-D def_base_dir=" + def_base_dir,
        # pass "def_file" global variable to cog script
        "-D def_file=" + infile,
        # pass "out_file" global variable to cog script
        "-D out_file=" + outfile,
        # outfile name
        "-o " + outfile,
        # cog template file
        template]

    # call generator on shell to generate an instance of the template
    subprocess.check_call(" ".join(shell_args), shell=True)


def generate_fbs(flatc, infile, outdir):
    # assemble shell arguments
    shell_args = [
        # quote to avoid platform issues
        '"' + flatc + '"',
        # generate c++
        "--cpp",
        # include directory (of definition files)
        "-I " + os.path.dirname(infile),
        # output the cpp files
        "-o " + outdir,
        # fbs definition file
        infile]

    # call generator on shell to generate an instance of the template
    subprocess.check_call(" ".join(shell_args), shell=True)


def list_files(gen_dir, out_dir):
    """
    Collects all definition files from gendir and outputs the filenames in outdir they will have when generated.

    :return List of dicts with src, template and out keys
    """
    result = []

    for generator in generators:
        # this generators relative paths
        gen_rel = rel_definitions.format(generator=generator)
        gen_rel_template = rel_template.format(generator=generator)
        gen_rel_generator = rel_generator.format(generator=generator)

        # definitions of this generator
        gen_def_dir = os.path.join(gen_dir, gen_rel)
        # template file of this generator
        gen_template = os.path.abspath(os.path.join(os.path.dirname(__file__), gen_rel_template))
        # python file of this generator
        gen_generator = os.path.abspath(os.path.join(os.path.dirname(__file__), gen_rel_generator))
        # outputs of this generator
        gen_out_dir = os.path.join(out_dir, gen_rel)

        # walk all definitions
        for dir_path, _, files in os.walk(gen_def_dir):
            for file in files:
                def_file = os.path.abspath(os.path.join(dir_path, file))
                def_file_base, ext = os.path.splitext(def_file)
                if ext in allowed_exts:
                    # make def path relative to def dir
                    def_rel_base = os.path.relpath(def_file_base, gen_def_dir)

                    # transfer it to out dir
                    out_file = os.path.join(gen_out_dir, def_rel_base) + ".h"

                    # add to result
                    result.append({
                        'src': def_file,
                        'template': gen_template,
                        'generator': gen_generator,
                        'out': out_file
                    })

                    if generator == "flatbuffers":
                        result[-1]['fbs'] = os.path.join(gen_out_dir, def_rel_base) + ".fbs"
                        result[-1]['outdir'] = os.path.dirname(out_file)

    return result


if __name__ == "__main__":
    # at least a verb and two paths must be supplied
    if len(argv) < 4:
        raise Exception("Invalid arguments")

    # extract args
    _verb, _gen_dir, _out_dir = argv[1], argv[2], argv[3]

    # all source files, their templates and respective output files
    file_list = list_files(_gen_dir, _out_dir)

    if _verb == "list":
        # only list the output files, don't actually do anything
        # also, use CMake list format "a;b;c"
        print(";".join(f['out'] for f in file_list), end="")

    elif _verb == "depend":
        # build a list of all the dependencies of the output files
        dep_files = \
            [f['src'] for f in file_list] + \
            [f['template'] for f in file_list] + \
            [f['generator'] for f in file_list]
        # also, use CMake list format "a;b;c"
        print(";".join(dep_files), end="")

    elif _verb == "generate":
        # actually generate the files
        for f in file_list:
            generate(_gen_dir, f['src'], f['template'], f['out'])

            # optionally generate fbs
            if "fbs" in f:
                generate_fbs(argv[4], f['fbs'], f['outdir'])

    else:
        raise Exception("Unknown verb")
