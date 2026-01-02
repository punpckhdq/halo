# ninja file and solution generator for csplit

import io
import json
import math
import os
import platform
import sys
from enum import Enum
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

from . import ninja_syntax
from .ninja_syntax import serialize_path

from .vsgen.configuration import BuildParams
from .vsgen.configuration import Configuration
from .vsgen.sln import SolutionFile
from .vsgen.vcxproj import VcxProj

class ObjectStatus(Enum):
    Missing = 0
    NonMatching = 1
    Matching = 2

class Object:
    def __init__(self, completed: ObjectStatus, file_path: Path, **options: Any) -> None:
        self.file_path = file_path
        self.status = completed
        self.options: Dict[str, Any] = {
            "cflags": None,
            "include_dirs": None,
            "defines": None,
        }
        self.options.update(options)

        # Internal
        self.base_obj_path: Optional[Path] = None
        self.split_obj_path: Optional[Path] = None

    def resolve(self, sln: "SolutionConfig", proj: "ProjectConfig") -> "Object":
        # Use object options, then library options
        obj = Object(self.status, self.file_path, **proj.options)
        for key, value in self.options.items():
            if value is not None or key not in obj.options:
                obj.options[key] = value

        # Resolve paths
        base_name = self.file_path.with_suffix("")
        obj.base_obj_path = sln.build_dir / "base" / f"{base_name}.obj"
        obj.split_obj_path = sln.build_dir / "split" / f"{base_name}.obj"
        return obj

class ProjectConfig:
    def __init__(self, **options: Any) -> None:
        self.name: Optional[str] = None
        self.guid: Optional[str] = None
        self.objects: Optional[List[Object]] = None
        self.options: Dict[str, Any] = {
            "cflags": None,
            "include_dirs": None,
            "headers": None,
            "defines": None,
        }
        self.options.update(options)

    def validate(self) -> None:
        required_attrs = [
            "name",
            "guid",
            "objects",
        ]
        for attr in required_attrs:
            if getattr(self, attr) is None:
                sys.exit(f"ProjectConfig.{attr} missing")

    # Creates a map of object names to Object instances
    def resolve(self, sln: "SolutionConfig") -> Dict[str, Object]:
        out = {}
        for obj in self.objects:
            name = str(obj.file_path.with_suffix("")).replace(os.sep, "/")
            if name in out:
                sys.exit(f"Duplicate object name {name}")
            out[name] = obj.resolve(sln, self)
        return out

class SolutionConfig:
    def __init__(self) -> None:
        self.build_dir: Optional[Path] = None
        self.config_dir: Optional[Path] = None
        self.project_dir: Optional[Path] = None
        self.tools_dir: Optional[Path] = None
        self.name: Optional[str] = None
        self.guid: Optional[str] = None
        self.projects: Optional[List[ProjectConfig]] = None

        # Tooling
        self.ninja_path: Optional[Path] = None  # If None, use system PATH
        self.csplit_tag: Optional[str] = None  # Git tag
        self.csplit_path: Optional[Path] = None  # If None, download
        self.objdiff_tag: Optional[str] = None  # Git tag
        self.objdiff_path: Optional[Path] = None  # If None, download
        self.wibo_tag: Optional[str] = None # Git tag
        self.wrapper: Optional[Path] = None  # If None, download wibo on Linux
        
        # Project config
        self.baserom: Optional[Path] = None
        
        # Progress output and report.json config
        self.progress = True  # Enable report.json generation and CLI progress output
        self.print_progress_categories: bool = True
        self.progress_report_args: Optional[List[str]] = (
            None  # Flags to `objdiff-cli report generate`
        )

        # Progress fancy printing
        self.progress_use_fancy: bool = False
        self.progress_code_fancy_frac: int = 0
        self.progress_code_fancy_item: str = ""
        self.progress_data_fancy_frac: int = 0
        self.progress_data_fancy_item: str = "0"

    def validate(self) -> None:
        required_attrs = [
            "build_dir",
            "config_dir",
            "project_dir",
            "tools_dir",
            "name",
            "guid",
            "projects",
            "baserom",
        ]
        for attr in required_attrs:
            if getattr(self, attr) is None:
                sys.exit(f"SolutionConfig.{attr} missing")
        for proj in self.projects:
            proj.validate()

    # Gets the wrapper to use for compiler commands, if set.
    def compiler_wrapper(self) -> Optional[Path]:
        wrapper = self.wrapper

        if self.use_wibo():
            wrapper = self.build_dir / "tools" / "wibo"
        if not is_windows() and wrapper is None:
            wrapper = Path("wine")

        return wrapper

    # Determines whether or not to use wibo as the compiler wrapper.
    def use_wibo(self) -> bool:
        return (
            self.wibo_tag is not None
            and (sys.platform == "linux" or sys.platform == "darwin")
            and platform.machine() in ("i386", "x86_64", "aarch64", "arm64")
            and self.wrapper is None
        )

def is_windows() -> bool:
    return os.name == "nt"

# On Windows, we need this to use && in commands
CHAIN = "cmd /c " if is_windows() else ""
# Native executable extension
EXE = ".exe" if is_windows() else ""

# Generate all build files
def generate_build(sln: SolutionConfig) -> None:
    sln.validate()
    generate_build_ninja(sln)
    generate_objdiff_config(sln)
    generate_compile_commands(sln)
    if is_windows():
        generate_solution(sln)

# Generate build.ninja
def generate_build_ninja(sln: SolutionConfig) -> None:
    out = io.StringIO()
    n = ninja_syntax.Writer(out)
    n.variable("ninja_required_version", "1.3")
    n.newline()

    configure_script = Path(os.path.relpath(os.path.abspath(sys.argv[0])))
    python_lib = Path(os.path.relpath(__file__))
    python_lib_dir = python_lib.parent
    n.comment("The arguments passed to configure.py, for rerunning it.")
    n.variable("configure_args", sys.argv[1:])
    n.variable("python", f'"{sys.executable}"')
    n.newline()
    
    ###
    # Tooling
    ###
    n.comment("Tooling")
    
    report_path = sln.build_dir / "report.json"
    build_tools_path = sln.build_dir / "tools"
    download_tool = sln.tools_dir / "download_tool.py"
    n.rule(
        name="download_tool",
        command=f"$python {download_tool} $tool $out --tag $tag",
        description="TOOL $out",
    )

    if sln.objdiff_path:
        objdiff = sln.objdiff_path
    elif sln.objdiff_tag:
        objdiff = build_tools_path / f"objdiff-cli{EXE}"
        n.build(
            outputs=objdiff,
            rule="download_tool",
            implicit=download_tool,
            variables={
                "tool": "objdiff-cli",
                "tag": sln.objdiff_tag,
            },
        )
    else:
        sys.exit("SolutionConfig.objdiff_tag missing")

    if sln.csplit_path:
        csplit = sln.csplit_path
    elif sln.csplit_tag:
        csplit = build_tools_path / f"csplit{EXE}"
        n.build(
            outputs=csplit,
            rule="download_tool",
            implicit=download_tool,
            variables={
                "tool": "csplit",
                "tag": sln.csplit_tag,
            },
        )
    else:
        sys.exit("SolutionConfig.csplit_tag missing")

    wrapper = sln.compiler_wrapper()
    if wrapper is not None and sln.use_wibo():
        n.build(
            outputs=wrapper,
            rule="download_tool",
            implicit=download_tool,
            variables={
                "tool": "wibo",
                "tag": sln.wibo_tag,
            },
        )

    wrapper_implicit: Optional[Path] = None
    if wrapper is not None and (wrapper.exists() or sln.use_wibo()):
        wrapper_implicit = wrapper

    wrapper_cmd = f"{wrapper} " if wrapper else ""

    n.newline()

    ###
    # Helper rule for downloading all tools
    ###
    n.comment("Download all tools")
    n.build(
        outputs="tools",
        rule="phony",
        inputs=[objdiff, csplit],
    )
    n.newline()
    
    ###
    # Build rules
    ###
    n.rule(
        name="cl",
        command=f"{wrapper_cmd}xbox/bin/vc7/CL.Exe /nologo /c $cflags /Fo$out $in",
        description="CL $out",
    )
    n.newline()
    
    ###
    # Source files / projects
    ###
    n.comment("Projects")
    n.newline()
    
    base_object_targets: List[Path] = []
    split_object_targets: List[Path] = []
    for proj in sln.projects:
        objects: Dict[str, Object] = proj.resolve(sln)
        n.comment(proj.name)
        proj_base_object_targets: List[Path] = []
        for obj_name, obj in objects.items():
            split_object_targets.append(obj.split_obj_path)
            if obj.status != ObjectStatus.Missing:
                proj_base_object_targets.append(obj.base_obj_path)
                cflags: List[str] = []
                cflags.extend(obj.options["cflags"])
                cflags.extend([f"/D{define}" for define in obj.options["defines"]])
                cflags.extend([f"/I\"{path}\"" for path in obj.options["include_dirs"]])
                n.build(
                    outputs=obj.base_obj_path,
                    rule="cl",
                    variables={"cflags": cflags},
                    inputs=obj.file_path,
                    implicit=wrapper_implicit
                )
        base_object_targets.extend(proj_base_object_targets)
        n.build(
            outputs=f"{proj.name}_build",
            rule="phony",
            inputs=proj_base_object_targets,
        )
        n.newline()

    ###
    # Helper rule for building all source files
    ###
    n.comment("Build all source files")
    n.build(
        outputs="all_source",
        rule="phony",
        inputs=base_object_targets,
    )
    n.newline()

    ###
    # Build all split objects
    ###
    n.comment("Build all split files")
    n.build(
        outputs="all_split",
        rule="phony",
        inputs=split_object_targets,
    )
    n.newline()

    ###
    # Split EXE
    ###
    n.rule(
        name="csplit",
        command=f"{csplit} -i $in -p $config_dir -o $out_dir",
    )
    n.build(
        outputs=split_object_targets,
        rule="csplit",
        variables={
            "out_dir": sln.build_dir / "split",
            "config_dir": sln.config_dir
        },
        inputs=sln.baserom,
        implicit=[
            csplit,
            sln.config_dir / "config.json",
            sln.config_dir / "contribs.json",
            sln.config_dir / "relocs.json",
            sln.config_dir / "splits.json",
            sln.config_dir / "symbols.json",
        ],
    )
    n.newline()

    ###
    # Calculate progress
    ###
    n.comment("Calculate progress")
    n.rule(
        name="progress",
        command=f"$python {configure_script} $configure_args progress",
        description="PROGRESS",
    )
    n.build(
        outputs="progress",
        rule="progress",
        implicit=[
            configure_script,
            python_lib,
            report_path,
        ],
    )
    n.newline()

    ###
    # Generate progress report
    ###
    n.comment("Generate progress report")
    n.rule(
        name="report",
        command=f"{objdiff} report generate $objdiff_report_args -o $out",
        description="REPORT",
    )
    n.build(
        outputs=report_path,
        rule="report",
        implicit=[objdiff, "objdiff.json", "all_source", "all_split"],
    )
    n.newline()

    ###
    # Regenerate on change
    ###
    n.comment("Reconfigure on change")
    n.rule(
        name="configure",
        command=f"$python {configure_script} $configure_args",
        generator=True,
        description=f"RUN {configure_script}",
    )
    n.build(
        outputs=["build.ninja", "objdiff.json"],
        rule="configure",
        implicit=[
            sln.config_dir / "config.json",
            configure_script,
            python_lib,
            python_lib_dir / "ninja_syntax.py",
        ],
    )
    n.newline()

    ###
    # Default rule
    ###
    n.comment("Default rule")
    if sln.progress:
        n.default("progress")
    else:
        n.default("all_source")

    # Write build.ninja
    with open("build.ninja", "w", encoding="utf-8") as f:
        f.write(out.getvalue())
    out.close()

# Generate objdiff.json
def generate_objdiff_config(sln: SolutionConfig) -> None:
    if sln.ninja_path:
        ninja = str(sln.ninja_path.absolute())
    else:
        ninja = "ninja"
    
    objdiff_config: Dict[str, Any] = {
        "min_version": "2.0.0-beta.5",
        "custom_make": "ninja",
        "build_target": True,
        "watch_patterns": [
            "*.c",
            "*.cc",
            "*.cp",
            "*.cpp",
            "*.cxx",
            "*.c++",
            "*.h",
            "*.hh",
            "*.hp",
            "*.hpp",
            "*.hxx",
            "*.h++",
            "*.pch",
            "*.pch++",
            "*.inc",
            "*.py",
            "*.yml",
            "*.txt",
            "*.json",
        ],
        "units": [],
        "progress_categories": [],
    }

    for proj in sln.projects:
        objdiff_config["progress_categories"].append(
            {
                "id": proj.name,
                "name": proj.name,
            }
        )
        for obj_name, obj in proj.resolve(sln).items():
            unit = {
                "name": obj_name,
                "target_path": obj.split_obj_path,
                "metadata": {
                    "progress_categories": [ proj.name ],
                }
            }
            if obj.status != ObjectStatus.Missing:
                unit["base_path"] = obj.base_obj_path
                unit["metadata"]["source_path"] = obj.file_path
            if obj.status == ObjectStatus.Matching:
                unit["metadata"]["complete"] = True
            objdiff_config["units"].append(unit)

    def cleandict(d):
        if isinstance(d, dict):
            return {k: cleandict(v) for k, v in d.items() if v is not None}
        elif isinstance(d, list):
            return [cleandict(v) for v in d]
        else:
            return d

    # Write objdiff.json
    with open("objdiff.json", "w", encoding="utf-8") as w:
        def unix_path(input: Any) -> str:
            return str(input).replace(os.sep, "/") if input else ""
        json.dump(cleandict(objdiff_config), w, indent=2, default=unix_path)

# Generate compile_commands.json
def generate_compile_commands(sln: SolutionConfig) -> None:
    pass

# Generate solution and projects
def generate_solution(sln: SolutionConfig) -> None:
    relative_root = Path("..")
    vc_config = Configuration("Debug", "Win32", False, "", relative_root)
    vc_projects: List[VcxProj] = []
    for proj in sln.projects:
        vc_proj = VcxProj(proj.name, sln.project_dir)
        vc_proj.uuid = proj.guid
        vc_proj.add_config(vc_config)
        
        vc_macros: List[str] = proj.options["defines"]
        vc_include_dirs: List[Path] = [relative_root / path for path in proj.options["include_dirs"] or []]
        vc_params = BuildParams(macros=vc_macros, include_directories=vc_include_dirs, output=f"{proj.name}_build")
        vc_proj.add_build_params(str(vc_config), vc_params)
        
        vc_sources: List[Path] = []
        for obj in proj.resolve(sln).values():
            if obj.status != ObjectStatus.Missing:
                vc_sources.append(relative_root / obj.file_path)
        vc_proj.add_sources(vc_sources)
        
        vc_headers: List[Path] = [relative_root / header for header in proj.options["headers"] or []]
        vc_proj.add_headers(vc_headers)
        
        vc_extras: List[Path] = [
            relative_root / sln.config_dir / "config.json",
            relative_root / sln.config_dir / "symbols.json",
        ]
        vc_proj.add_extra_files(vc_extras)
        
        vc_proj.write()
        vc_projects.append(vc_proj)
    vc_sln = SolutionFile(sln.project_dir / sln.name)
    vc_sln.uuid = sln.guid
    for vc_proj in vc_projects:
        vc_sln.add_project(vc_proj, build_solution_target=True)
    vc_sln.write()

# Print progress information from objdiff report
def calculate_progress(sln: SolutionConfig) -> None:
    sln.validate()
    report_path = sln.build_dir / "report.json"
    if not report_path.is_file():
        sys.exit(f"Report file {report_path} does not exist")

    report_data: Dict[str, Any] = {}
    with open(report_path, "r", encoding="utf-8") as f:
        report_data = json.load(f)

    # Convert string numbers (u64) to int
    def convert_numbers(data: Dict[str, Any]) -> None:
        for key, value in data.items():
            if isinstance(value, str) and value.isdigit():
                data[key] = int(value)

    convert_numbers(report_data["measures"])
    for category in report_data.get("categories", []):
        convert_numbers(category["measures"])

    # Output to GitHub Actions job summary, if available
    summary_path = os.getenv("GITHUB_STEP_SUMMARY")
    summary_file: Optional[IO[str]] = None
    if summary_path:
        summary_file = open(summary_path, "a", encoding="utf-8")
        summary_file.write("```\n")

    def progress_print(s: str) -> None:
        print(s)
        if summary_file:
            summary_file.write(s + "\n")

    # Print human-readable progress
    progress_print("Progress:")

    def print_category(name: str, measures: Dict[str, Any]) -> None:
        total_code = measures.get("total_code", 0)
        matched_code = measures.get("matched_code", 0)
        matched_code_percent = measures.get("matched_code_percent", 0)
        total_data = measures.get("total_data", 0)
        matched_data = measures.get("matched_data", 0)
        matched_data_percent = measures.get("matched_data_percent", 0)
        total_functions = measures.get("total_functions", 0)
        matched_functions = measures.get("matched_functions", 0)
        complete_code_percent = measures.get("complete_code_percent", 0)
        total_units = measures.get("total_units", 0)
        complete_units = measures.get("complete_units", 0)

        progress_print(
            f"  {name}: {matched_code_percent:.2f}% matched, {complete_code_percent:.2f}% linked ({complete_units} / {total_units} files)"
        )
        progress_print(
            f"    Code: {matched_code} / {total_code} bytes ({matched_functions} / {total_functions} functions)"
        )
        progress_print(
            f"    Data: {matched_data} / {total_data} bytes ({matched_data_percent:.2f}%)"
        )

    print_category("All", report_data["measures"])
    for category in report_data.get("categories", []):
        if sln.print_progress_categories is True or (
            isinstance(sln.print_progress_categories, list)
            and category["id"] in sln.print_progress_categories
        ):
            print_category(category["name"], category["measures"])

    if sln.progress_use_fancy:
        measures = report_data["measures"]
        total_code = measures.get("total_code", 0)
        total_data = measures.get("total_data", 0)
        if total_code == 0 or total_data == 0:
            return
        code_frac = measures.get("complete_code", 0) / total_code
        data_frac = measures.get("complete_data", 0) / total_data

        progress_print(
            "\nYou have {} out of {} {} and {} out of {} {}.".format(
                math.floor(code_frac * sln.progress_code_fancy_frac),
                sln.progress_code_fancy_frac,
                sln.progress_code_fancy_item,
                math.floor(data_frac * sln.progress_data_fancy_frac),
                sln.progress_data_fancy_frac,
                sln.progress_data_fancy_item,
            )
        )

    # Finalize GitHub Actions job summary
    if summary_file:
        summary_file.write("```\n")
        summary_file.close()
