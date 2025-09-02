#!/usr/bin/env python3
"""
find_ncm.py - Find NCM files with colorful logging matching ncmpp format
"""

import sys
import os
from pathlib import Path

# Import color logging
sys.path.append(str(Path(__file__).parent))
try:
    from color_log import ColorLogger
    log = ColorLogger.log
    info = ColorLogger.info
    warn = ColorLogger.warn
    error = ColorLogger.error
    success = ColorLogger.success
except ImportError:
    # Fallback without colors
    def log(msg, level="INFO"): print(f"[{level}] {msg}")
    info = lambda msg: log(msg, "INFO")
    warn = lambda msg: log(msg, "WARN")
    error = lambda msg: log(msg, "ERROR")
    success = lambda msg: log(msg, "SUCCESS")

def generate_ncm_lists(music_dir):
    """
    Scans a directory for .ncm files and generates input and output lists for ncmpp.
    """
    music_path = Path(music_dir)

    if not music_path.is_dir():
        error(f"Directory not found at '{music_dir}'")
        return

    info(f"Scanning for .ncm files in '{ColorLogger.path(music_path.resolve())}'...")

    ncm_files = list(music_path.rglob('*.ncm'))

    if not ncm_files:
        warn(f"No .ncm files found in '{ColorLogger.path(music_path.resolve())}'.")
        return

    output_dir = Path.cwd()
    input_list_path = output_dir / 'ncm_input.txt'
    output_list_path = output_dir / 'ncm_output.txt'

    with open(input_list_path, 'w', encoding='utf-8') as f_in, \
         open(output_list_path, 'w', encoding='utf-8') as f_out:
        for ncm_file in ncm_files:
            # Write the full path to the input file
            f_in.write(str(ncm_file.resolve()) + '\n')

            # Write the desired output path (without .ncm extension) to the current directory
            # Use stem to preserve full filename including dots
            base_name = ncm_file.stem
            f_out.write(str(ncm_file.parent / base_name) + '\n')

    success(f"Generated '{ColorLogger.path(input_list_path.name)}' and '{ColorLogger.path(output_list_path.name)}' in '{ColorLogger.path(output_dir)}'")
    success(f"Found {len(ncm_files)} .ncm files.")

if __name__ == "__main__":
    if len(sys.argv) > 2:
        error("Usage: python find_ncm.py [directory]")
        sys.exit(1)

    target_directory = sys.argv[1] if len(sys.argv) == 2 else "."
    generate_ncm_lists(target_directory)