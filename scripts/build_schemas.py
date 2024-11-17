#!/usr/bin/python
# Copyright (c) 2021-present Sparky Studios. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Builds all Amplitude flatbuffer schemas to binary modules.

Generates binary schema files (.bfbs) bundled in the SDK release.
"""

import argparse
import glob
import os
import platform
import subprocess
import sys


def find_in_paths(name, paths):
    """Searches for a file with named `name` in the given paths and returns it."""
    for path in paths:
        full_path = os.path.join(path, name)
        if os.path.isfile(full_path):
            return full_path
    # If not found, just assume it's in the PATH.
    return name


# The project root directory, which is one level up from this script's
# directory.
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__),
                                            os.path.pardir))

# Directories that may contain the FlatBuffers compiler.
FLATBUFFERS_PATHS = [
    os.path.join(PROJECT_ROOT, 'bin'),
    os.path.join(PROJECT_ROOT, 'bin', 'Release'),
    os.path.join(PROJECT_ROOT, 'bin', 'Debug')
]

# Windows uses the .exe extension on executables.
EXECUTABLE_EXTENSION = '.exe' if platform.system() == 'Windows' else ''

# Name of the flatbuffer executable.
FLATC_EXECUTABLE_NAME = 'flatc' + EXECUTABLE_EXTENSION

# Location of FlatBuffers compiler.
FLATC = find_in_paths(FLATC_EXECUTABLE_NAME, FLATBUFFERS_PATHS)


class BuildError(Exception):
    """Error indicating there was a problem building assets."""

    def __init__(self, argv, error_code, message=None):
        Exception.__init__(self)
        self.argv = argv
        self.error_code = error_code
        self.message = message if message else ''


def run_subprocess(argv):
    try:
        process = subprocess.Popen(argv)
    except OSError as e:
        raise BuildError(argv, 1, message=str(e))
    process.wait()
    if process.returncode:
        raise BuildError(argv, process.returncode)


def compile_flatbuffer_binary_schema(flatc, schema, out_dir):
    """Run the flatbuffer compiler on the given schema file.

    Args:
      flatc: Path to the flatc binary.
      schema: The path to the schema.
      out_dir: The directory to write the flatbuffer binary schema.

    Raises:
      BuildError: Process return code was nonzero.
    """
    command = [flatc, '-o', out_dir]
    command.extend(['-I', os.path.join(PROJECT_ROOT, 'schemas')])
    command.extend(['-b', schema, '--schema'])
    run_subprocess(command)


def generate_flatbuffer_binaries(flatc, target_directory):
    """Run the flatbuffer compiler on the all the flatbuffer schema files.

    Args:
      flatc: Path to the flatc binary.
      target_directory: Path to the target assets directory.
    """
    for schema in glob.glob(os.path.join(PROJECT_ROOT, 'schemas', '*.fbs'), recursive=False):
        compile_flatbuffer_binary_schema(flatc, schema, target_directory)


def handle_build_error(error):
    """Prints an error message to stderr for BuildErrors."""
    sys.stderr.write(
        "Error running command `%s`. Returned %s.\n%s\n"
        % (" ".join(error.argv), str(error.error_code), str(error.message))
    )


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument('--flatc', default=FLATC,
                        help='Location of the flatbuffers compiler.')
    parser.add_argument('--output', default=os.path.join(PROJECT_ROOT, 'schemas'),
                        help='Assets output directory.')
    parser.add_argument('args', nargs=argparse.REMAINDER)
    args = parser.parse_args()
    target = args.args[1] if len(args.args) >= 2 else 'all'

    try:
        generate_flatbuffer_binaries(args.flatc, args.output)
    except BuildError as error:
        handle_build_error(error)
        return 1

    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
