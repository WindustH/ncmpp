#!/usr/bin/env python3
"""
ncmpp.py - All-in-one NCM processing tool

Usage: python ncmpp.py /path/to/music/directory

This script:
1. Finds .ncm files recursively
2. Generates input and output lists
3. Runs ncmpp binary to convert files
4. Embeds cover images into converted files
5. Cleans up cover images
"""

import sys
import os
import subprocess
from pathlib import Path

from typing import Any
ColorLogger: Any = None
# Import color logging
sys.path.append(str(Path(__file__).parent))
try:
    # Prefer importing the module so we don't introduce a conflicting class
    import color_log as _color_log
    ColorLogger = _color_log.ColorLogger
    log = ColorLogger.log
    info = ColorLogger.info
    warn = ColorLogger.warn
    error = ColorLogger.error
    success = ColorLogger.success
except Exception:
    # Fallback without colors — provide a minimal fallback class
    class _FallbackColorLogger:
        @staticmethod
        def path(p):
            return str(p)

        @staticmethod
        def log(msg, level="INFO"):
            print(f"[{level}] {msg}")

        @staticmethod
        def info(msg):
            _FallbackColorLogger.log(msg, "INFO")

        @staticmethod
        def warn(msg):
            _FallbackColorLogger.log(msg, "WARN")

        @staticmethod
        def error(msg):
            _FallbackColorLogger.log(msg, "ERROR")

        @staticmethod
        def success(msg):
            _FallbackColorLogger.log(msg, "SUCCESS")

    ColorLogger = _FallbackColorLogger
    # Expose the same helpers the real ColorLogger provides
    log = ColorLogger.log
    info = ColorLogger.info
    warn = ColorLogger.warn
    error = ColorLogger.error
    success = ColorLogger.success


def find_ncm_files(music_dir):
    """Find .ncm files recursively and generate input/output lists."""
    music_path = Path(music_dir)

    if not music_path.is_dir():
        error(f"Directory not found at '{ColorLogger.path(music_dir)}'")
        return None, None

    info(f"Scanning for .ncm files in '{ColorLogger.path(music_path.resolve())}'...")

    ncm_files = list(music_path.rglob('*.ncm'))

    if not ncm_files:
        info(f"No .ncm files found in '{ColorLogger.path(music_path.resolve())}'.")
        return None, None

    # Create temporary files for ncmpp
    temp_dir = Path.cwd() / "ncmpp_temp"
    temp_dir.mkdir(exist_ok=True)

    input_list_path = temp_dir / "ncm_input.txt"
    output_list_path = temp_dir / "ncm_output.txt"

    with open(input_list_path, 'w', encoding='utf-8') as f_in, \
         open(output_list_path, 'w', encoding='utf-8') as f_out:
        for ncm_file in ncm_files:
            # Write the full path to the input file
            f_in.write(str(ncm_file.resolve()) + '\n')

            # Write the desired output path (without .ncm extension)
            # Use stem to preserve full filename including dots
            base_name = ncm_file.stem
            output_path = ncm_file.parent / base_name
            f_out.write(str(output_path) + '\n')

    success(f"Found {len(ncm_files)} .ncm files.")
    return input_list_path, output_list_path


def run_ncmpp(input_file, output_file):
    """Run the ncmpp binary to convert files."""
    info("Running ncmpp to convert files...")

    try:
        # Check if ncmpp is in PATH
        try:
            subprocess.run(["ncmpp", "--help"], capture_output=True, check=True)
            ncmpp_cmd = "ncmpp"
            info("Using ncmpp from PATH")
        except (subprocess.CalledProcessError, FileNotFoundError):
            # Use local ncmpp
            ncmpp_path = Path(__file__).parent / "build" / "ncmpp"
            if not ncmpp_path.exists():
                error("ncmpp binary not found in PATH or ./build/ncmpp")
                return False
            ncmpp_cmd = str(ncmpp_path)
            info(f"Using local ncmpp: {ColorLogger.path(ncmpp_path)}")

        cmd = [
            ncmpp_cmd,
            "-i", str(input_file),
            "-o", str(output_file),
            "-s"  # Show timing
        ]

        info(f"Executing: {' '.join(cmd)}")
        result = subprocess.run(cmd, capture_output=True, text=True)

        if result.returncode == 0:
            success("ncmpp conversion completed successfully!")
            if result.stdout:
                info(result.stdout)
            return True
        else:
            error("Error running ncmpp:")
            error(result.stderr)
            return False

    except Exception as e:
        error(f"Error running ncmpp: {e}")
        return False


def run_embed_cover(output_file):
    """Run embed_cover.py to embed covers."""
    info("Running embed_cover.py to embed cover images...")

    try:
        # Check if we're in the right environment
        if Path("ncmpp_env").exists():
            activate_script = Path("ncmpp_env") / "bin" / "activate"
            cmd = f"source {activate_script} && python embed_cover.py {output_file}"
            info("Using virtual environment for embed_cover.py")
            result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        else:
            cmd = [sys.executable, "embed_cover.py", str(output_file)]
            info(f"Executing: {' '.join(cmd)}")
            result = subprocess.run(cmd, capture_output=True, text=True)

        if result.returncode == 0:
            success("Cover embedding completed successfully!")
            if result.stdout:
                info(result.stdout)
            return True
        else:
            error("Error running embed_cover.py:")
            error(result.stderr)
            return False

    except Exception as e:
        error(f"Error running embed_cover.py: {e}")
        return False


def cleanup_temp_files(temp_dir):
    """Clean up temporary files."""
    try:
        if temp_dir.exists():
            for file in temp_dir.iterdir():
                file.unlink()
            temp_dir.rmdir()
            success(f"Cleaned up temporary files in {temp_dir}")
    except Exception as e:
        warn(f"Could not clean up temp files: {e}")


def main():
    """Main function."""
    if len(sys.argv) != 2:
        error("Usage: python ncmpp.py /path/to/music/directory")
        sys.exit(1)

    music_dir = sys.argv[1]

    info("=== NCM All-in-One Processing Tool ===")
    info(f"Processing directory: {ColorLogger.path(music_dir)}")

    # Step 1: Find NCM files and generate lists
    input_file, output_file = find_ncm_files(music_dir)
    if not input_file or not output_file:
        sys.exit(1)

    # Step 2: Run ncmpp to convert files
    if not run_ncmpp(input_file, output_file):
        error("Conversion failed. Skipping cover embedding.")
        cleanup_temp_files(input_file.parent)
        sys.exit(1)

    # Step 3: Run embed_cover.py to embed covers
    if not run_embed_cover(output_file):
        error("Cover embedding failed.")

    # Step 4: Clean up
    cleanup_temp_files(input_file.parent)

    success("=== Processing Complete ===")


if __name__ == "__main__":
    main()
