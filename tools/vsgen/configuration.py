from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path


@dataclass(eq=True, unsafe_hash=True)
class BaseConfig:
    name: str
    arch: str

    def __str__(self) -> str:
        return f'{self.name}|{self.arch}'

    @classmethod
    def from_str(cls, config: str) -> BaseConfig:
        return cls(*config.split('|'))

    @property
    def condition(self) -> str:
        return f"'$(Configuration)|$(Platform)'=='{self!s}'"

    def __lt__(self, other: Configuration) -> bool:
        return (self.name, self.arch) < (other.name, other.arch)


@dataclass(eq=True)
class Configuration(BaseConfig):

    is_debug: bool
    toolset: str
    builddir: Path
    use_env_include: bool = False


@dataclass
class BuildParams:

    output: str | None = None
    include_directories: list[Path] = field(default_factory=list)
    macros: list[str] = field(default_factory=list)
    additional_options: list[str] = field(default_factory=list)
    search_paths: list[Path] = field(default_factory=list)
    envs: dict[str, str] = field(default_factory=dict)
