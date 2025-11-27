from __future__ import annotations

from .configuration import BaseConfig
from .vcxproj import VcxProj

from dataclasses import dataclass, field
from pathlib import Path
import re
from typing import Optional
import uuid


@dataclass
class ProjectData:

    uuid: str
    name: str
    path: Path

    configurations: set[BaseConfig] = field(default_factory=set)
    subdir: Optional[str] = None
    build_solution_target: bool = False

    def __post_init__(self):
        if self.subdir:
            self.subdir = self.subdir.strip("/")

    def __lt__(self, other: ProjectData) -> bool:
        return self.name < other.name


class SolutionFile:

    VCXPROJ_UUID = "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"
    SUBDIR_UUID = "{2150E333-8FDC-42A3-9474-1A3956D46DE8}"

    UUID_RE = r'\{[A-F0-9]{8}-[A-F0-9]{4}-[A-F0-9]{4}-[A-F0-9]{4}-[A-F0-9]{12}\}'

    def __init__(self, solution_path: Path):
        self.path = solution_path.with_suffix('.sln')

        self.projects: dict[str, ProjectData] = {}

        self._subdirs: dict[str, str] = {}  # map path to uuid
        self.uuid = f"{{{str(uuid.uuid4())}}}".upper()

    def add_project(self, project: VcxProj, subdir: Optional[str] = None, build_solution_target: bool = False) -> None:
        project_path = project.filepath
        if project_path.is_relative_to(self.path.parent):
            project_path = project_path.relative_to(self.path.parent)

        project_data = ProjectData(
            project.uuid, project.name, project_path, subdir=subdir, build_solution_target=build_solution_target
        )

        if subdir:
            self._add_subdir(subdir.strip('/'))

        project_data.configurations.update(BaseConfig(c.name, c.arch) for c in project.configs.values())
        self.projects[project.uuid] = project_data

    def _add_subdir(self, subdir: str) -> None:
        parts = subdir.split('/')
        while parts:
            subpath = '/'.join(parts)
            if subpath not in self._subdirs:
                self._subdirs[subpath] = f"{{{str(uuid.uuid5(uuid.NAMESPACE_URL, subpath))}}}".upper()
            parts.pop(-1)

    def load_configs(self) -> None:
        if not self.path.exists():
            return

        contents = self.path.read_text(encoding='utf-8')
        m = re.search(f'SolutionGuid = ({self.UUID_RE})', contents)
        self.uuid = m[1]

        for m in re.finditer(rf'Project\("{self.VCXPROJ_UUID}"\) = "(.*)", "(.*)", "({self.UUID_RE})"', contents):
            project_path = Path(m[2])
            if (self.path.parent / project_path).with_suffix(VcxProj.EXT).exists():
                project_data = ProjectData(m[3], m[1], project_path)
                self.projects[project_data.uuid] = project_data

        sln_subdirs = {}  # map subdir uuid to subdir name
        for m in re.finditer(rf'Project\("{self.SUBDIR_UUID}"\) = "(.*)", "(.*)", "({self.UUID_RE})"', contents):
            sln_subdirs[m[3]] = m[1]

        # rebuild solution hieararchy...
        sln_subdirs_parents = {}
        prj_subdirs = {}  # map project uuid to subdir uuid
        for m in re.finditer(f'({self.UUID_RE}) = ({self.UUID_RE})', contents):
            if m[1] in sln_subdirs:
                sln_subdirs_parents[m[1]] = m[2]
            elif m[1] in self.projects:
                prj_subdirs[m[1]] = m[2]

        while sln_subdirs:
            top_level = {u for u in sln_subdirs if u not in sln_subdirs_parents}

            for u, p in list(sln_subdirs_parents.items()):
                if p in top_level:
                    sln_subdirs[u] = sln_subdirs[p] + '/' + sln_subdirs[u]
                    del sln_subdirs_parents[u]

            for t in top_level:
                sd = sln_subdirs.pop(t)
                self._subdirs[sd] = t
                
                for p, u in list(prj_subdirs.items()):
                    if u == t:
                        self.projects[p].subdir = sd
                        del prj_subdirs[p]
        assert not sln_subdirs_parents

        for m in re.finditer(r'(\{[0-9A-F-]+\}).([\w-]+\|[\w]+)\.ActiveCfg', contents):
            self.projects[m[1]].configurations.add(BaseConfig.from_str(m[2]))
        for m in re.finditer(r'(\{[0-9A-F-]+\}).([\w-]+\|[\w]+)\.Build\.0', contents):
            self.projects[m[1]].build_solution_target = True

    def write(self) -> None:
        # Reference: https://learn.microsoft.com/en-us/visualstudio/extensibility/internals/solution-dot-sln-file?view=vs-2022  # noqa
        contents = [  # FIXME: versions here are totally arbitrary...
            'Microsoft Visual Studio Solution File, Format Version 12.00',
            '# Visual Studio Version 17',
            'VisualStudioVersion = 17.8.34330.188',
            'MinimumVisualStudioVersion = 10.0.40219.1',
        ]

        configurations = set()
        for project in sorted(self.projects.values()):
            contents.append(f'Project("{self.VCXPROJ_UUID}") = "{project.name}", "{project.path}", "{project.uuid}"')
            contents.append('EndProject')
            configurations.update(project.configurations)
        for subdir in sorted(self._subdirs):
            subdir_uuid = self._subdirs[subdir]
            parts = subdir.split('/')
            contents.append(f'Project("{self.SUBDIR_UUID}") = "{parts[-1]}", "{parts[-1]}", "{subdir_uuid}"')
            contents.append('EndProject')

        contents.append('Global')

        contents.append('	GlobalSection(SolutionConfigurationPlatforms) = preSolution')
        for config in sorted(configurations):
            contents.append(f'		{config} = {config}')
        contents.append('	EndGlobalSection')

        contents.append('	GlobalSection(ProjectConfigurationPlatforms) = postSolution')
        for project in sorted(self.projects.values()):
            for config in sorted(project.configurations):
                contents.append(f'		{project.uuid}.{config}.ActiveCfg = {config}')
                if project.build_solution_target:
                    contents.append(f'		{project.uuid}.{config}.Build.0 = {config}')
        contents.append('	EndGlobalSection')

        contents.append('	GlobalSection(SolutionProperties) = preSolution')
        contents.append('		HideSolutionNode = FALSE')
        contents.append('	EndGlobalSection')

        if self._subdirs:
            contents.append('	GlobalSection(NestedProjects) = preSolution')

            for project in sorted(self.projects.values()):
                if project.subdir:
                    subdir_uuid = self._subdirs[project.subdir]
                    contents.append(f'		{project.uuid} = {subdir_uuid}')

            for subdir_name, subdir_uuid in sorted(self._subdirs.items()):
                if '/' in subdir_name:
                    parent, _ = subdir_name.rsplit('/', maxsplit=1)
                    parent_uuid = self._subdirs[parent]
                    contents.append(f'		{subdir_uuid} = {parent_uuid}')
            contents.append('	EndGlobalSection')

        contents.append('	GlobalSection(ExtensibilityGlobals) = postSolution')
        contents.append(f'		SolutionGuid = {self.uuid}')
        contents.append('	EndGlobalSection')

        contents.append('EndGlobal')

        self.path.write_text('\n'.join(contents), encoding='utf-8')
