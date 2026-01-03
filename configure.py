#!/usr/bin/env python3

# root build script

import argparse
import json
from pathlib import Path
from typing import (
    IO,
    Any,
    Callable,
    Dict,
    Iterable,
    List,
    Optional,
    Set,
    Tuple,
    TypedDict,
    Union,
    cast,
)

from tools.project_x86 import (
    Object,
    ObjectStatus,
    ProjectConfig,
    SolutionConfig,
    calculate_progress,
    generate_build,
    is_windows,
)
import os
# arguments
parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY",
    type=Path,
    help="path to objdiff-cli binary (optional)",
)
parser.add_argument(
    "--csplit",
    metavar="BINARY",
    type=Path,
    help="path to csplit binary (optional)",
)
parser.add_argument(
    "--ninja",
    metavar="BINARY",
    type=Path,
    help="path to ninja binary (optional)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )

args = parser.parse_args()

# path setup

build_dir = Path("build")
config_dir = Path("config")
config_json_path = config_dir / "config.json"
project_dir = Path("projects")
tools_dir = Path("tools")

# build config json

class BuildConfigObject(TypedDict):
    name: str
    index: int
    status: str
    options: Dict[str, Any]

class BuildConfigProject(TypedDict):
    name: str
    guid: str
    objects: List[BuildConfigObject]
    options: Dict[str, Any]

class BuildConfig(TypedDict):
    name: str
    guid: str
    baserom: str
    projects: List[BuildConfigProject]

build_config: BuildConfig = json.load(open(config_json_path, "r", encoding="utf-8"))

# solution configuration

sln = SolutionConfig()
sln.build_dir = build_dir
sln.config_dir = config_dir
sln.project_dir = project_dir
sln.tools_dir = tools_dir

sln.name = build_config["name"]
sln.guid = build_config["guid"]
sln.baserom = build_config["baserom"]

# Apply arguments
sln.objdiff_path = args.objdiff
sln.csplit_path = args.csplit
sln.ninja_path = args.ninja
if not is_windows():
    sln.wrapper = args.wrapper

# Tool versions
sln.objdiff_tag = "v3.3.1"
sln.csplit_tag = "v0.0.2"
sln.wibo_tag = "1.0.0"

sln.projects = []
for build_project in build_config["projects"]:
    project = ProjectConfig(**build_project.get("options", {}))
    project.name = build_project["name"]
    project.guid = build_project["guid"]
    project.objects = []
    for build_object in build_project["objects"]:
        def get_object_completed(status: str) -> ObjectStatus:
            if status == "MISSING":
                return ObjectStatus.Missing
            elif status == "Matching":
                return ObjectStatus.Matching
            elif status == "NonMatching":
                return ObjectStatus.NonMatching
            assert False, f"Invalid object status {status}"
        project.objects.append(Object(get_object_completed(build_object["status"]), Path(build_object["name"]), **build_object.get("options", {})))
    sln.projects.append(project)

# build file generation

if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(sln)
elif args.mode == "progress":
    # Print progress information
    calculate_progress(sln)
else:
    sys.exit("Unknown mode: " + args.mode)
