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
    schema_path: str = os.path.join(SDK_PATH, "schemas")

    def __init__(self, argv, script_name: str, script_version: str):
        opts, args = getopt.getopt(argv, "hvp:b:f:s:",
                                   ["help", "version", "project-path", "build-path", "flatc", "schema-path", "no-logo"])

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
            elif opt in ("-s", "--schema-path"):
                self.schema_path = arg

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


def convert_json_to_flatbuffers_binary(flatc: str, json: str, schema: str, out_dir: str, schema_path: str):
    """
    Convert a JSON file to a FlatBuffers binary using the specified schema.

    This function runs the FlatBuffers compiler (flatc) to convert a given JSON file
    into a FlatBuffers binary format using the provided schema.

    Args:
        flatc (str): Path to the FlatBuffers compiler (flatc) executable.
        json (str): Path to the JSON file to be converted.
        schema (str): Path to the FlatBuffers schema file (.fbs) to use for conversion.
        out_dir (str): Directory where the output FlatBuffers binary will be written.
        schema_path (str): Path to the directory containing additional schema files.

    Raises:
        BuildError: If the FlatBuffers compiler process returns a non-zero exit code,
                    indicating a failure in the conversion process.

    Note:
        This function does not return any value. The resulting FlatBuffers binary
        is written to the specified output directory.
    """

    run_subprocess([flatc, "-o", out_dir, "-I", schema_path, "-b", schema, json])

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


def generate_flatbuffers_binaries(options: CommandOptions):
    """
    Generate FlatBuffers binary files from JSON files using the specified schemas.

    This function iterates through the conversion data, which includes schema and input file
    information, and converts each JSON file to a FlatBuffers binary. It checks if the output
    directory exists, creates it if necessary, and only rebuilds files that have been modified
    since the last build or if the schema has changed.

    Args:
        options (CommandOptions): An object containing command-line options and paths.
            It includes the following attributes:
            - flatc_path: Path to the FlatBuffers compiler executable.
            - project_path: Path to the directory containing the Amplitude project.
            - build_path: Path to the directory where the FlatBuffers binaries will be generated.
            - schema_path: Path to the directory containing the schema files.

    Raises:
        BuildError: If the FlatBuffers compilation process returns a non-zero exit code,
                    indicating a failure in the conversion process.

    Note:
        This function does not return any value. The resulting FlatBuffers binaries
        are written to the specified output directory.
    """

    flatc = options.flatc_path
    conversion_data = get_conversion_data(options.project_path, [options.schema_path])
    input_path = options.project_path
    output_path = options.build_path

    for element in conversion_data:
        schema = element.schema
        for json in element.input_files:
            target = processed_json_path(json, input_path, output_path)
            target_file_dir = os.path.dirname(target)
            if not os.path.exists(target_file_dir):
                os.makedirs(target_file_dir)
            if needs_rebuild(json, target) or needs_rebuild(schema, target):
                convert_json_to_flatbuffers_binary(flatc, json, schema, target_file_dir, options.schema_path)


def find_in_paths(name: str, paths: list[str]) -> str:
    """
    Search for a file with the given name in the specified paths and return its full path.

    This function iterates through the provided paths, checking for the existence of a file
    with the given name. If found, it returns the full path to the file. If the file is not
    found in any of the specified paths, the function assumes the file might be in the system's
    PATH and returns just the filename.

    Args:
        name (str): The name of the file to search for.
        paths (list[str]): A list of directory paths to search in.

    Returns:
        str: The full path to the file if found in one of the specified paths,
             or just the filename if not found (assuming it might be in the system's PATH).
    """
    for path in paths:
        full_path = os.path.join(path, name)
        if os.path.isfile(full_path):
            return full_path
    # If not found, just assume it's in the PATH.
    return name


def clean_flatbuffers_binaries(options: CommandOptions):
    """
    Deletes all the processed FlatBuffers binary files generated from JSON files.

    This function iterates through all the FlatBuffers binary files that were generated
    from JSON files in the Amplitude project and removes them from the build directory.
    It uses the project path and build path specified in the CommandOptions to locate
    and delete these files.

    Args:
        options (CommandOptions): An object containing command-line options and paths.
            It includes the following relevant attributes:
            - project_path: Path to the directory containing the Amplitude project.
            - build_path: Path to the directory where the FlatBuffers binaries were generated.

    Returns:
        None

    Note:
        This function does not return any value but has the side effect of deleting files
        from the file system.
    """
    conversion_data = get_conversion_data(options.project_path, [options.schema_path])
    input_path = options.project_path
    output_path = options.build_path

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


def get_conversion_data(project_path: str, schema_paths: list[str]) -> list[FlatbuffersConversionData]:
    """
    Retrieves conversion data for Flatbuffers compilation of JSON files in an Amplitude project.

    This function gathers information about various JSON files in the Amplitude project
    directory and their corresponding Flatbuffers schema files. It creates
    FlatbuffersConversionData objects for different types of audio-related data,
    including engine configurations, buses, sound banks, collections, sounds, events,
    pipelines, attenuators, switches, switch containers, RTPCs, and effects.

    Args:
        project_path (str): The path to the Amplitude project directory containing
                            the JSON files to be converted.
        schema_paths (list[str]): A list of paths where the Flatbuffers schema
                                  files (.bfbs) can be found.

    Returns:
        list[FlatbuffersConversionData]: A list of FlatbuffersConversionData objects,
        each containing the schema file path and a list of input JSON files for a
        specific type of audio data. This information is used to convert the JSON
        files to Flatbuffers binary format.
    """
    return [
        FlatbuffersConversionData(
            schema=find_in_paths(
                'engine_config_definition.bfbs', schema_paths),
            input_files=glob.glob(os.path.join(project_path, '*.config.json'))),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'buses_definition.bfbs', schema_paths),
            input_files=glob.glob(os.path.join(project_path, '*.buses.json'))),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'sound_bank_definition.bfbs', schema_paths),
            input_files=glob.glob(os.path.join(project_path, SOUNDBANKS_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'collection_definition.bfbs', schema_paths),
            input_files=glob.glob(os.path.join(project_path, COLLECTIONS_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'sound_definition.bfbs', schema_paths),
            input_files=glob.glob(os.path.join(project_path, SOUNDS_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'event_definition.bfbs', schema_paths),
            input_files=glob.glob(os.path.join(project_path, EVENTS_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'pipeline_definition.bfbs', schema_paths),
            input_files=glob.glob(os.path.join(project_path, PIPELINES_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'attenuation_definition.bfbs', schema_paths),
            input_files=glob.glob(os.path.join(project_path, ATTENUATORS_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'switch_definition.bfbs', schema_paths),
            input_files=glob.glob(os.path.join(project_path, SWITCHES_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'switch_container_definition.bfbs', schema_paths),
            input_files=glob.glob(os.path.join(project_path, SWITCH_CONTAINERS_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'rtpc_definition.bfbs', schema_paths),
            input_files=glob.glob(os.path.join(project_path, RTPC_DIR_NAME, '**/*.json'), recursive=True)),
        FlatbuffersConversionData(
            schema=find_in_paths(
                'effect_definition.bfbs', schema_paths),
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
