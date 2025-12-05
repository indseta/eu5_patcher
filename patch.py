"""EU5 Patcher - Enable Achievements Unconditionally"""

import re
import shutil
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Final

# Constants
# Achievement Check
PATTERN: Final[str] = (
    "80 ?? ?? ?? ?? ?? 00 75 ?? "
    "80 ?? ?? ?? ?? ?? 00 74 ?? "
    "80 ?? ?? ?? ?? ?? 00 74 ?? "
    "80 ?? ?? ?? ?? ?? 00"
)

PATTERN_REPLACE: Final[str] = "80 ?? ?? ?? ?? ?? 00 eb"

# Ironman Save and Load
PATTERN2: Final[str] = (
    "74 ?? "
    "?? ?? ?? "
    "ff ?? ?? "
    "e8 ?? ?? ?? ?? "
    "80 ?? ?? ?? ?? ?? 00 "
    "74 ?? "
    "32 c0"
)

PATTERN_REPLACE2: Final[str] = (
    "eb ?? "
    "?? ?? ?? "
    "ff ?? ?? "
    "e8 ?? ?? ?? ?? "
    "80 ?? ?? ?? ?? ?? 00 "
    "74 ?? "
    "b0 00"
)

# Ironman Console
PATTERN3: Final[str] = (
    "00 "
    "75 ?? "
    "80 ?? ?? ?? ?? ?? 00 "
    "75 ?? "
    "32 c0 "
    "48 ?? ?? ?? "
    "c3 "
    "b0 01"
)

PATTERN_REPLACE3: Final[str] = (
    "00 "
    "75 ?? "
    "80 ?? ?? ?? ?? ?? 00 "
    "75 ?? "
    "32 c0 "
    "48 ?? ?? ?? "
    "c3 "
    "b0 00"
)

# Save
PATTERN4: Final[str] = (
    "e8 ?? ?? ?? ?? "
    "80 ?? ?? ?? ?? ?? 00 "
    "75 ?? "
    "80 ?? ?? ?? ?? ?? 00 "
    "75 ?? "
    "48 ?? ?? "
    "e8"
)

PATTERN_REPLACE4: Final[str] = "e8 ?? ?? ?? ?? " "80 ?? ?? ?? ?? ?? 09"

# Load
PATTERN5: Final[str] = (
    "80 ?? ?? ?? ?? ?? 00 "
    "75 ?? "
    "80 ?? ?? ?? ?? ?? 00 "
    "75 ?? "
    "83 ?? ?? ?? ?? ?? 00 "
    "75 ?? "
    "48 ?? ?? "
    "e8"
)

PATTERN_REPLACE5: Final[str] = "80 ?? ?? ?? ?? ?? 09"


EU5_PATH: Final[Path] = Path("eu5.exe")
EU5_BACKUP_PATH: Final[Path] = Path("eu5.exe.backup")

debuge_info = False


@dataclass(frozen=True)
class PatchDefinition:
    label: str
    pattern: str
    replacement: str


PATCHES: Final[list[PatchDefinition]] = [
    PatchDefinition("Patch #1", PATTERN, PATTERN_REPLACE),
    PatchDefinition("Patch #2", PATTERN2, PATTERN_REPLACE2),
    PatchDefinition("Patch #3", PATTERN3, PATTERN_REPLACE3),
    PatchDefinition("Patch #4", PATTERN4, PATTERN_REPLACE4),
    PatchDefinition("Patch #5", PATTERN5, PATTERN_REPLACE5),
]


class PatchError(Exception):
    """Custom exception for patch-related errors."""


def pattern_to_regex(pattern_str: str) -> re.Pattern[bytes]:
    """Convert a hex pattern string with wildcards to a compiled regex."""
    parts = pattern_str.split()
    regex_parts: list[bytes] = []
    wildcard_count = 0

    for part in parts:
        if part == "??":
            wildcard_count += 1
        else:
            if wildcard_count > 0:
                regex_parts.append(f".{{{wildcard_count}}}".encode())
                wildcard_count = 0
            # Escape the byte in case it's a regex special char
            regex_parts.append(re.escape(bytes.fromhex(part)))

    if wildcard_count > 0:
        regex_parts.append(f".{{{wildcard_count}}}".encode())

    return re.compile(b"".join(regex_parts), re.DOTALL)


def pattern_to_list(pattern_str: str) -> list[int | None]:
    """Convert a hex pattern string to a list of byte values (None for wildcards)."""
    return [None if part == "??" else int(part, 16) for part in pattern_str.split()]


def create_backup(source: Path, dest: Path) -> None:
    """Create a backup of the source file."""
    try:
        shutil.copy2(source, dest)
        print(f"Backup created: {dest}")
    except OSError as e:
        raise PatchError(f"Failed to create backup: {e}") from e


def find_pattern(data: bytes, pattern: re.Pattern[bytes]) -> int:
    """Find the pattern in data and return its offset."""
    matches = list(pattern.finditer(data))

    if len(matches) == 0:
        raise PatchError(
            "Pattern not found. Have you patched it before? "
            "If not, the pattern may need to be updated."
        )
    if len(matches) > 1:
        raise PatchError(
            f"Multiple matches found ({len(matches)}). " "The pattern is ambiguous."
        )

    return matches[0].start()


def apply_patch(data: bytearray, offset: int, replacement: list[int | None]) -> None:
    """Apply the replacement pattern at the specified offset."""
    for i, value in enumerate(replacement):
        original = data[offset + i]
        if value is None:
            if debuge_info:
                print(
                    f"{offset + i:#x}: {original:#04x} -> {original:#04x} (unchanged)"
                )
        else:
            if debuge_info:
                print(f"{offset + i:#x}: {original:#04x} -> {value:#04x}")
            data[offset + i] = value


def make_patch(filepath: Path) -> None:
    """Patch the target executable."""
    # Read file
    try:
        data = bytearray(filepath.read_bytes())
    except OSError as e:
        raise PatchError(f"Failed to read file: {e}") from e

    # Find every pattern and prepare replacements before touching disk
    patch_jobs: list[tuple[PatchDefinition, int, list[int | None]]] = []
    for patch_def in PATCHES:
        regex = pattern_to_regex(patch_def.pattern)
        offset = find_pattern(data, regex)
        replacement = pattern_to_list(patch_def.replacement)
        patch_jobs.append((patch_def, offset, replacement))

    # Create backup only after both patterns are confirmed
    create_backup(filepath, EU5_BACKUP_PATH)

    for patch_def, offset, replacement in patch_jobs:
        print(f"\n{patch_def.label} found at offset: {offset:#x}")
        if debuge_info:
            print(f"Applying {patch_def.label}...\n")
        apply_patch(data, offset, replacement)

    # Write back
    try:
        filepath.write_bytes(data)
        print("\nEU5 is successfully patched.")
    except OSError as e:
        raise PatchError(f"Failed to write file: {e}") from e


def main() -> int:
    """Main entry point."""
    if not EU5_PATH.exists():
        print(
            "eu5.exe not found. "
            "Place this script in .../Europa Universalis V/binaries/"
        )
        input("Press Enter to exit...")
        return 1

    try:
        make_patch(EU5_PATH)
    except PatchError as e:
        print(f"Error: {e}")
        input("Press Enter to exit...")
        return 1

    input("Press Enter to exit...")
    return 0


if __name__ == "__main__":
    sys.exit(main())
