from __future__ import annotations

from collections.abc import Iterable
import os.path
from pathlib import Path
import re
from typing import Optional
import uuid

import xml.etree.ElementTree as ET

from .configuration import Configuration, BuildParams


class VcxProj:

    NS: str = 'http://schemas.microsoft.com/developer/msbuild/2003'
    EXT = '.vcxproj'
    FILTER_EXT = EXT + '.filters'
    USER_EXT = EXT + '.user'

    def __init__(self, name: str, path: Path):
        self.name: str = name
        self.path: Path = path
        self.uuid: str = f'{{{str(uuid.uuid4())}}}'.upper()

        self.configs: dict[str, Configuration] = {}
        self.params: dict[str, BuildParams] = {}

        self._files: dict[Path, str] = {}

    @property
    def filepath(self) -> Path:
        return self.path / (self.name + self.EXT)

    @property
    def filterspath(self) -> Path:
        return self.path / (self.name + self.FILTER_EXT)

    @property
    def userpath(self) -> Path:
        return self.path / (self.name + self.USER_EXT)

    def _load_configs(self, project_xml: ET.Element):
        self.uuid = project_xml.find(f'.//{{{self.NS}}}ProjectGuid').text

        for config_node in project_xml.iterfind(f'.//{{{self.NS}}}ProjectConfiguration'):
            config = config_node.findtext(f"{{{self.NS}}}Configuration")
            arch = config_node.findtext(f"{{{self.NS}}}Platform")

            bc = Configuration(config, arch, False, "", Path())
            self.configs[str(bc)] = bc
            self.params[str(bc)] = BuildParams()

        for config_node in project_xml.iterfind(f'.//{{{self.NS}}}PropertyGroup'):
            condition = config_node.get("Condition")
            config = None
            for bc in self.configs.values():
                if bc.condition == condition:
                    config = bc
                    params = self.params[str(bc)]
                    break

            if config is None:
                continue

            label = config_node.get('Label')

            if label == 'UserMacros':
                config.builddir = Path(config_node.find(f'{{{self.NS}}}BuildDir').text)
                if search_paths := self._read_opt_node(config_node, 'DllPaths'):
                    params.search_paths = list(map(Path, search_paths.split(';')))

            elif label == "Configuration":
                config.toolset = self._read_opt_node(config_node, 'PlatformToolset')
                config.is_debug = config_node.find(f'{{{self.NS}}}UseDebugLibraries').text == 'true'

            elif label is None:
                if include_directories := self._read_opt_node(config_node, 'NMakeIncludeSearchPath'):
                    params.include_directories = list(map(Path, include_directories.split(';')))
                if macros := self._read_opt_node(config_node, 'NMakePreprocessorDefinitions'):
                    params.macros = macros.split(' ')
                if additional_options := self._read_opt_node(config_node, 'AdditionalOptions'):
                    params.additional_options = additional_options.split(';')
                if self._read_opt_node(config_node, 'IncludePath') == '$(INCLUDE)':
                    config.use_env_include = True

                build_cmd = self._read_opt_node(config_node, 'NMakeBuildCommandLine', '')
                m = re.fullmatch(r'ninja -C (\S+) (\S+)', build_cmd)
                params.output = m[2] if m else None

                # TODO: LocalDebuggerEnvironment

    def _load_files(self, project_xml: ET.Element):
        for filetype in ("ClCompile", "ClInclude", "None"):
            for file_node in project_xml.iterfind(f'.//{{{self.NS}}}{filetype}'):
                self._files[Path(self.filepath.parent, file_node.get("Include")).resolve()] = filetype

    def load(self, required=True) -> None:
        if not self.filepath.exists():
            if required:
                raise FileNotFoundError(f'{self.filepath} does not exist')
            else:
                return

        project_xml = ET.parse(self.filepath).getroot()
        self._load_configs(project_xml)
        self._load_files(project_xml)

    def _read_opt_node(self, parent: ET.Element, name: str, default: Optional[str] = None) -> str | None:
        node = parent.find(f'{{{self.NS}}}{name}')
        if node is None:
            return default
        return node.text or default

    def remove_user_file(self) -> None:
        self.userpath.unlink(missing_ok=True)

    def add_config(self, config: Configuration):
        self.configs[str(config)] = config

    def add_build_params(self, config: str, params: BuildParams):
        self.params[config] = params

    def write(self) -> VcxProj:
        self.path.mkdir(parents=True, exist_ok=True)
        self._write_project_file()
        self._write_filters_file()
        if not self.userpath.exists():
            self._write_user_file()
        return self  # for chaining

    def _xml_project(self, **kwargs: str) -> ET.Element:
        nsmap = {None: self.NS}
        return ET.Element('Project', nsmap=nsmap, attrib=kwargs)

    @staticmethod
    def _write_xml(path: Path, xml_root: ET.Element) -> None:
        path.write_bytes(ET.tostring(xml_root, encoding='UTF-8', xml_declaration=True))

    def _write_project_file(self) -> None:
        root = self._xml_project(DefaultTargets='Build')

        project_configurations = ET.SubElement(root, 'ItemGroup', attrib={'Label': 'ProjectConfigurations'})
        for config in sorted(self.configs.values()):
            pc = ET.SubElement(project_configurations, 'ProjectConfiguration', attrib={'Include': str(config)})
            ET.SubElement(pc, 'Configuration').text = config.name
            ET.SubElement(pc, 'Platform').text = config.arch

        globals_group = ET.SubElement(root, 'PropertyGroup', attrib={'Label': 'Globals'})
        ET.SubElement(globals_group, 'VCProjectVersion').text = '15.0'
        ET.SubElement(globals_group, 'ProjectGuid').text = self.uuid
        ET.SubElement(globals_group, 'Keyword').text = 'MakeFileProj'

        ET.SubElement(root, 'Import', attrib={'Project': '$(VCTargetsPath)\\Microsoft.Cpp.Default.props'})

        for config in sorted(self.configs.values()):
            conf = ET.SubElement(
                root, 'PropertyGroup', attrib={'Label': 'Configuration', 'Condition': config.condition}
            )
            ET.SubElement(conf, 'ConfigurationType').text = 'Makefile'
            ET.SubElement(conf, 'UseDebugLibraries').text = str(config.is_debug).lower()
            ET.SubElement(conf, 'PlatformToolset').text = config.toolset

        ET.SubElement(root, 'Import', attrib={'Project': '$(VCTargetsPath)\\Microsoft.Cpp.props'})
        ET.SubElement(root, 'ImportGroup', attrib={'Label': 'ExtensionSettings'})
        ET.SubElement(root, 'ImportGroup', attrib={'Label': 'Shared'})

        for config in sorted(self.configs.values()):
            prop_sheets = ET.SubElement(
                root, 'ImportGroup', attrib={'Label': 'PropertySheets', 'Condition': config.condition}
            )
            ET.SubElement(
                prop_sheets,
                'Import',
                attrib={
                    'Label': 'LocalAppDataPlatform',
                    'Project': '$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props',
                    'Condition': "exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')",
                },
            )

        for config in sorted(self.configs.values()):
            params = self.params[str(config)]

            user_macros = ET.SubElement(
                root, 'PropertyGroup', attrib={'Label': 'UserMacros', 'Condition': config.condition}
            )
            ET.SubElement(user_macros, 'BuildDir').text = self._relpath(config.builddir)
            ET.SubElement(user_macros, 'DllPaths').text = ';'.join(map(str, params.search_paths or []))
            #     self._relpath(p, config.builddir) for p in params.search_paths
            # )

        for config in sorted(self.configs.values()):
            params = self.params[str(config)]

            compile_cmd = ET.SubElement(root, 'PropertyGroup', attrib={'Condition': config.condition})
            if params.output is not None:
                ET.SubElement(compile_cmd, 'NMakeBuildCommandLine').text = f'ninja -C "$(BuildDir)" {params.output}'
                if params.output:
                    ET.SubElement(compile_cmd, 'NMakeOutput').text = self._relpath(config.builddir / params.output)
                ET.SubElement(
                    compile_cmd, 'NMakeReBuildCommandLine'
                ).text = f'ninja -C "$(BuildDir)" clean && ninja -C "$(BuildDir)" {params.output}'
            ET.SubElement(compile_cmd, 'NMakeCleanCommandLine').text = 'ninja -C "$(BuildDir)" clean'
            if params.macros:
                ET.SubElement(compile_cmd, 'NMakePreprocessorDefinitions').text = ';'.join(params.macros)
            if params.include_directories:
                ET.SubElement(compile_cmd, 'NMakeIncludeSearchPath').text = ';'.join(
                    map(self._relpath, params.include_directories)
                )
            if params.additional_options:
                ET.SubElement(compile_cmd, 'AdditionalOptions').text = ' '.join(params.additional_options)
            ET.SubElement(compile_cmd, 'LocalDebuggerWorkingDirectory').text = '$(BuildDir)'

            env = params.envs.copy()
            if 'PATH' in env:
                env['PATH'] = env['PATH'] + ';$(DllPaths)'
            else:
                env['PATH'] = '$(DllPaths)'
            ET.SubElement(compile_cmd, 'LocalDebuggerEnvironment').text = '\n'.join(f'{k}={v}' for k, v in env.items())

            if config.use_env_include:
                ET.SubElement(compile_cmd, 'IncludePath').text = '$(INCLUDE)'

        cat_nodes: dict[str, ET.Element] = {}
        for filename, category in sorted(self._files.items()):
            node = cat_nodes.get(category)
            if node is None:
                node = ET.SubElement(root, 'ItemGroup')
                cat_nodes[category] = node
            ET.SubElement(node, category, attrib={'Include': self._relpath(filename)})

        ET.SubElement(root, 'Import', attrib={'Project': '$(VCTargetsPath)\\Microsoft.Cpp.targets'})
        ET.SubElement(root, 'ImportGroup', attrib={'Label': 'ExtensionTargets'})

        self._write_xml(self.filepath, root)

    def _find_source_dir(self) -> str | None:
        try:
            source_dir = os.path.commonpath(self._files)
            if os.path.isfile(source_dir):
                return os.path.dirname(source_dir)
            return source_dir
        except ValueError:
            return None

    def _write_filters_file(self) -> None:
        source_dir = self._find_source_dir()

        def _dirname(file: Path) -> str:
            return os.path.dirname(os.path.relpath(file, source_dir)) if source_dir else str(file.parent)

        subdirs = set()
        for file in sorted(self._files):
            dirname = _dirname(file)
            if dirname:
                subdirs.add(dirname)
                for p in Path(dirname).parents:
                    if str(p) != '.' and str(p) not in subdirs:
                        subdirs.add(str(p))
                        continue
                    break

        root = self._xml_project(ToolsVersion='4.0')
        filter_list = ET.SubElement(root, "ItemGroup")
        for dirname in sorted(subdirs):
            ET.SubElement(filter_list, "Filter", attrib={"Include": dirname})

        file_list = ET.SubElement(root, "ItemGroup")
        for filename, category in sorted(self._files.items()):
            file_node = ET.SubElement(file_list, category, attrib={"Include": self._relpath(filename)})
            dirname = _dirname(filename)
            if dirname != '.':
                ET.SubElement(file_node, "Filter").text = dirname

        self._write_xml(self.filterspath, root)

    def _write_user_file(self) -> None:
        root = self._xml_project(ToolsVersion='Current')
        ET.SubElement(root, 'PropertyGroup')
        self._write_xml(self.userpath, root)

    def _relpath(self, f: Path, base: Path | None = None) -> str:
        if f.is_absolute():
            return os.path.relpath(f, base or self.path)
        else:
            return str(f)

    def add_sources(self, sources: Iterable[Path]) -> None:
        for f in sources:
            self._files[f] = 'ClCompile'

    def add_headers(self, headers: Iterable[Path]) -> None:
        for f in headers:
            self._files[f] = 'ClInclude'

    def add_extra_files(self, extras: Iterable[Path]) -> None:
        for f in extras:
            self._files[f] = 'None'
