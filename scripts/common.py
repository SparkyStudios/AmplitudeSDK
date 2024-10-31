# coding:utf-8
# !/usr/bin/python
#
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
#
import getopt
import glob
import os
import shutil
import subprocess
import sys
from pathlib import Path

# The path to the Amplitude Audio SDK
SDK_PATH = os.getenv("AM_SDK_PATH") or os.getcwd()

# The Amplitude Audio SDK schemas directory.
SCHEMA_PATHS = [
    os.path.join(SDK_PATH, "schemas")
]

# Name of the flatbuffers executable.
FLATC = (shutil.which("flatc")
         or os.path.join(SDK_PATH, "bin", os.getenv("AM_SDK_PLATFORM") or "", "flatc"))

# Directory where unprocessed sound flatbuffers data can be found.
SOUNDS_DIR_NAME = 'sounds'

# Directory where unprocessed collection flatbuffers data can be found.
COLLECTIONS_DIR_NAME = 'collections'

# Directory where unprocessed sound bank flatbuffers data can be found.
SOUNDBANKS_DIR_NAME = 'soundbanks'

# Directory where unprocessed event flatbuffers data can be found.
EVENTS_DIR_NAME = 'events'

# Directory where unprocessed pipeline flatbuffers data can be found.
PIPELINES_DIR_NAME = 'pipelines'

# Directory where unprocessed attenuation flatbuffers data can be found.
ATTENUATORS_DIR_NAME = 'attenuators'

# Directory where unprocessed switch flatbuffers data can be found.
SWITCHES_DIR_NAME = 'switches'

# Directory where unprocessed switch containers flatbuffers data can be found.
SWITCH_CONTAINERS_DIR_NAME = 'switch_containers'

# Directory where unprocessed rtpc flatbuffers data can be found.
RTPC_DIR_NAME = 'rtpc'

# Directory where unprocessed effect flatbuffers data can be found.
EFFECTS_DIR_NAME = 'effects'

# Directory where unprocessed environment flatbuffers data can be found.
ENVIRONMENTS_DIR_NAME = 'environments'


class FlatbuffersConversionData(object):
    """Holds data needed to convert a set of json files to flatbuffers binaries.

    Attributes:
      schema: The path to the flatbuffers schema file.
      input_files: A list of input files to convert.
    """

    def __init__(self, schema, input_files):
        """Initializes this object's schema and input_files."""
        self.schema = schema
        self.input_files = input_files


class BuildError(Exception):
    """Error indicating there was a problem building assets."""

    def __init__(self, argv, error_code, message=None):
        Exception.__init__(self)
        self.argv = argv
        self.error_code = error_code
        self.message = message if message else ""


class CommandOptions(object):
    """Holds the command line options."""

    project_path: str = None
    build_path: str = None
    flatc_path: str = FLATC

    def __init__(self, argv, script_name: str, script_version: str):
        opts, args = getopt.getopt(argv, "hvp:b:f:",
                                   ["help", "version", "project-path", "build-path", "flatc", "no-logo"])

        no_logo = False
        show_help = False
        show_version = False

        for opt, arg in opts:
            if opt == "--no-logo":
                no_logo = True
            elif opt in ("-h", "--help"):
                show_help = True
            elif opt in ("-v", "--version"):
                show_version = True
            elif opt in ("-p", "--project-path"):
                self.project_path = arg
            elif opt in ("-b", "--build-path"):
                self.build_path = arg
            elif opt in ("-f", "--flatc"):
                self.flatc_path = arg

        if show_help:
            print_help(no_logo, script_name)
            sys.exit(0)

        if show_version:
            print("{}.py {}".format(script_name, script_version))
            sys.exit(0)

        if self.project_path == '' or self.build_path == '':
            print_help(no_logo, script_name)
            sys.exit(1)

        # Check if running on Windows
        if sys.platform.startswith('win'):
            self.flatc_path = self.flatc_path.replace('/', '\\')
            if not self.flatc_path.lower().endswith(".exe"):
                self.flatc_path += ".exe"


def run_subprocess(argv):
    """
    Runs a subprocess with the given arguments.

    Args:
        argv (list): The arguments to pass to the subprocess.

    Raises:
        BuildError: If the subprocess returns a non-zero exit code.
    """
    try:
        process = subprocess.Popen(argv)
    except OSError as e:
        sys.stdout.write("Cannot find executable?")
        raise BuildError(argv, 1, message=str(e))
    process.wait()
    if process.returncode:
        sys.stdout.write("Process has exited with 1")
        raise BuildError(argv, process.returncode)


def convert_json_to_flatbuffers_binary(flatc, json, schema, out_dir):
    """Run the flatbuffers compiler on the given json file and schema.

    Args:
      flatc: Path to the flatc binary.
      json: The path to the json file to convert to a flatbuffers binary.
      schema: The path to the schema to use in the conversion process.
      out_dir: The directory to write the flatbuffers binary.

    Raises:
      BuildError: Process return code was nonzero.
    """
    command = [flatc, "-o", out_dir]
    for path in SCHEMA_PATHS:
        command.extend(["-I", path])
    command.extend(["-b", schema, json])
    run_subprocess(command)


def needs_rebuild(source, target):
    """Checks if the source file needs to be rebuilt.

    Args:
      source: The source file to be compared.
      target: The target file which we may need to rebuild.

    Returns:
      True if the source file is newer than the target, or if the target file
      does not exist.
    """
    return not os.path.isfile(target) or (
            os.path.getmtime(source) > os.path.getmtime(target)
    )


def processed_json_path(path: str, input_path: str, output_path: str):
    """Take the path to a raw json asset and convert it to target bin path."""
    return path.replace(
        ".json",
        ".amconfig" if path.endswith("config.json")
        else ".ambus" if path.endswith("buses.json")
        else ".ambank" if SOUNDBANKS_DIR_NAME in path
        else ".amcollection" if COLLECTIONS_DIR_NAME in path
        else ".amevent" if EVENTS_DIR_NAME in path
        else ".ampipeline" if PIPELINES_DIR_NAME in path
        else ".amattenuation" if ATTENUATORS_DIR_NAME in path
        else ".amswitch" if SWITCHES_DIR_NAME in path
        else ".amswitchcontainer" if SWITCH_CONTAINERS_DIR_NAME in path
        else ".amrtpc" if RTPC_DIR_NAME in path
        else ".amsound" if SOUNDS_DIR_NAME in path
        else ".amenv" if ENVIRONMENTS_DIR_NAME in path
        else ".ambin",
    ).replace(input_path, output_path)


def processed_json_filename(path: str, input_path: str, output_path: str):
    """Take the path to a raw json asset and return the filename of the binary asset."""
    return os.path.basename(processed_json_path(path, input_path, output_path))


def generate_flatbuffers_binaries(flatc: str, conversion_data: list[FlatbuffersConversionData], input_path: str,
                                  output_path: str):
    """
    Runs the flatbuffers compiler on the given json files and schemas.

    Args:
        flatc (str): Path to the flatc binary.
        conversion_data (list[FlatbuffersConversionData]): A list of conversion data, containing the schema and input
        files to convert.
        input_path (str): Path to the Amplitude project directory.
        output_path (str): Path to the output directory.

    Raises:
        BuildError: If the process returns a non-zero exit code.
    """
    for element in conversion_data:
        schema = element.schema
        for json in element.input_files:
            target = processed_json_path(json, input_path, output_path)
            target_file_dir = os.path.dirname(target)
            if not os.path.exists(target_file_dir):
                os.makedirs(target_file_dir)
            if needs_rebuild(json, target) or needs_rebuild(schema, target):
                convert_json_to_flatbuffers_binary(
                    flatc, json, schema, target_file_dir)


def find_in_paths(name, paths):
    """Searches for a file with named `name` in the given paths and returns it."""
    for path in paths:
        full_path = os.path.join(path, name)
        if os.path.isfile(full_path):
            return full_path
    # If not found, just assume it's in the PATH.
    return name


def clean_flatbuffers_binaries(conversion_data: list[FlatbuffersConversionData], input_path: str, output_path: str):
    """
    Deletes all the processed flatbuffers binary files.

    Args:
        conversion_data (list[FlatbuffersConversionData]): A list of conversion data, containing the schema and input
            files to convert.
        input_path (str): Path to the Amplitude project directory.
        output_path (str): Path to the output directory.
    """
    for element in conversion_data:
        for json in element.input_files:
            path = processed_json_path(json, input_path, output_path)
            if os.path.isfile(path):
                os.remove(path)


def handle_build_error(error):
    """Prints an error message to stderr for BuildErrors."""
    sys.stderr.write(
        "Error running command `%s`. Returned %s.\n%s\n"
        % (" ".join(error.argv), str(error.error_code), str(error.message))
    )


def get_amplitude_project_path():
    """
    Returns the path to the Amplitude project directory.

    This function looks for the AM_PROJECT_PATH environment variable, which is set when the project is opened in
    Amplitude Studio.

    Returns:
        The path to the Amplitude project directory, or None if it could not be found.
    """
    _AM_PROJECT_PATH = None
    try:
        _AM_PROJECT_PATH = Path(os.getenv('AM_PROJECT_PATH'))
    finally:
        # if None, fallback to engine folder
        if not _AM_PROJECT_PATH:
            sys.stderr.write("Unable to detect the Amplitude project root path.")

    return _AM_PROJECT_PATH


def get_conversion_data(project_path: str):
    """
    Returns a list of FlatbuffersConversionData objects that contain the necessary information to convert the
    json files to flatbuffers binaries.

    Args:
        project_path (str): The path to the Amplitude project directory.

    Returns:
        A list of FlatbuffersConversionData objects.
    """
    return [
        FlatbuffersConversionData(
            schema=find_in_paths(
                'engine_config_definition.bfbs', SCHEMA_PATHS),
            input_files=glob.glob(os.path.join(project_path, '*.config.json'))),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'buses_definition.bfbs', SCHEMA_PATHS),
            input_files=glob.glob(os.path.join(project_path, '*.buses.json'))),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'sound_bank_definition.bfbs', SCHEMA_PATHS),
            input_files=glob.glob(os.path.join(project_path, SOUNDBANKS_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'collection_definition.bfbs', SCHEMA_PATHS),
            input_files=glob.glob(os.path.join(project_path, COLLECTIONS_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'sound_definition.bfbs', SCHEMA_PATHS),
            input_files=glob.glob(os.path.join(project_path, SOUNDS_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'event_definition.bfbs', SCHEMA_PATHS),
            input_files=glob.glob(os.path.join(project_path, EVENTS_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'pipeline_definition.bfbs', SCHEMA_PATHS),
            input_files=glob.glob(os.path.join(project_path, PIPELINES_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'attenuation_definition.bfbs', SCHEMA_PATHS),
            input_files=glob.glob(os.path.join(project_path, ATTENUATORS_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'switch_definition.bfbs', SCHEMA_PATHS),
            input_files=glob.glob(os.path.join(project_path, SWITCHES_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'switch_container_definition.bfbs', SCHEMA_PATHS),
            input_files=glob.glob(os.path.join(project_path, SWITCH_CONTAINERS_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'rtpc_definition.bfbs', SCHEMA_PATHS),
            input_files=glob.glob(os.path.join(project_path, RTPC_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'effect_definition.bfbs', SCHEMA_PATHS),
            input_files=glob.glob(os.path.join(project_path, EFFECTS_DIR_NAME, '**/*.json'), recursive=True)),
    ]


def print_help(no_logo: bool = False, script_name: str = None):
    if not no_logo:
        print("Amplitude Audio SDK - Copyright (c) 2021-present Sparky Studios. All Rights Reserved.")
        print("========================\n")

    print("Usage:")
    print("  {}.py [options]\n".format(script_name))
    print("Options:")
    print("  -h, --help\t\t\tShows this help message.")
    print("  -v, --version\t\t\tShows the script version.")
    print("  --no-logo\t\t\t\tDisables the display of copyright header.")
    print("  -p, --project-path\tPath to the directory containing the Amplitude project.")
    print("  -b, --build-path\t\tPath to the directory where the flatbuffers binaries will be generated.")
    print("  --flatc\t\t\t\tPath to a custom flatc binary.")
