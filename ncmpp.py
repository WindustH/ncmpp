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


def find_ncm_files(music_dir):
    """Find .ncm files recursively and generate input/output lists."""
    music_path = Path(music_dir)
    
    if not music_path.is_dir():
        print(f"Error: Directory not found at '{music_dir}'")
        return None, None
    
    print(f"Scanning for .ncm files in '{music_path.resolve()}'...")
    
    ncm_files = list(music_path.rglob('*.ncm'))
    
    if not ncm_files:
        print(f"No .ncm files found in '{music_path.resolve()}'.")
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
    
    print(f"Found {len(ncm_files)} .ncm files.")
    return input_list_path, output_list_path


def run_ncmpp(input_file, output_file):
    """Run the ncmpp binary to convert files."""
    print("\nRunning ncmpp to convert files...")
    
    try:
        # Check if ncmpp is in PATH
        try:
            subprocess.run(["ncmpp", "--help"], capture_output=True, check=True)
            ncmpp_cmd = "ncmpp"
        except (subprocess.CalledProcessError, FileNotFoundError):
            # Use local ncmpp
            ncmpp_path = Path(__file__).parent / "build" / "ncmpp"
            if not ncmpp_path.exists():
                print("Error: ncmpp binary not found in PATH or ./build/ncmpp")
                return False
            ncmpp_cmd = str(ncmpp_path)
        
        cmd = [
            ncmpp_cmd,
            "-i", str(input_file),
            "-o", str(output_file),
            "-s"  # Show timing
        ]
        
        result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode == 0:
            print("ncmpp conversion completed successfully!")
            if result.stdout:
                print(result.stdout)
            return True
        else:
            print("Error running ncmpp:")
            print(result.stderr)
            return False
            
    except Exception as e:
        print(f"Error running ncmpp: {e}")
        return False


def run_embed_cover(output_file):
    """Run embed_cover.py to embed covers."""
    print("\nRunning embed_cover.py to embed cover images...")
    
    try:
        # Check if we're in the right environment
        if Path("ncmpp_env").exists():
            activate_script = Path("ncmpp_env") / "bin" / "activate"
            cmd = f"source {activate_script} && python embed_cover.py {output_file}"
            result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        else:
            cmd = [sys.executable, "embed_cover.py", str(output_file)]
            result = subprocess.run(cmd, capture_output=True, text=True)
        
        if result.returncode == 0:
            print("Cover embedding completed successfully!")
            print(result.stdout)
            return True
        else:
            print("Error running embed_cover.py:")
            print(result.stderr)
            return False
            
    except Exception as e:
        print(f"Error running embed_cover.py: {e}")
        return False


def cleanup_temp_files(temp_dir):
    """Clean up temporary files."""
    try:
        if temp_dir.exists():
            for file in temp_dir.iterdir():
                file.unlink()
            temp_dir.rmdir()
            print(f"Cleaned up temporary files in {temp_dir}")
    except Exception as e:
        print(f"Warning: Could not clean up temp files: {e}")


def main():
    """Main function."""
    if len(sys.argv) != 2:
        print("Usage: python ncmpp.py /path/to/music/directory")
        sys.exit(1)
    
    music_dir = sys.argv[1]
    
    print("=== NCM All-in-One Processing Tool ===")
    print(f"Processing directory: {music_dir}")
    
    # Step 1: Find NCM files and generate lists
    input_file, output_file = find_ncm_files(music_dir)
    if not input_file or not output_file:
        sys.exit(1)
    
    # Step 2: Run ncmpp to convert files
    if not run_ncmpp(input_file, output_file):
        print("Conversion failed. Skipping cover embedding.")
        cleanup_temp_files(input_file.parent)
        sys.exit(1)
    
    # Step 3: Run embed_cover.py to embed covers
    if not run_embed_cover(output_file):
        print("Cover embedding failed.")
    
    # Step 4: Clean up
    cleanup_temp_files(input_file.parent)
    
    print("\n=== Processing Complete ===")


if __name__ == "__main__":
    main()