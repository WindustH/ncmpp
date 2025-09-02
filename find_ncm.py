import sys
import os
from pathlib import Path

def generate_ncm_lists(music_dir):
    """
    Scans a directory for .ncm files and generates input and output lists for ncmpp.
    """
    music_path = Path(music_dir)

    if not music_path.is_dir():
        print(f"Error: Directory not found at '{music_dir}'")
        return

    print(f"Scanning for .ncm files in '{music_path.resolve()}'...")

    ncm_files = list(music_path.rglob('*.ncm'))

    if not ncm_files:
        print(f"No .ncm files found in '{music_path.resolve()}'.")
        return

    output_dir = Path.cwd()
    input_list_path = output_dir / 'ncm_input.txt'
    output_list_path = output_dir / 'ncm_output.txt'

    with open(input_list_path, 'w') as f_in, open(output_list_path, 'w') as f_out:
        for ncm_file in ncm_files:
            # Write the full path to the input file
            f_in.write(str(ncm_file.resolve()) + '\n')

            # Write the desired output path (without .ncm extension) to the current directory
            # Use stem to preserve full filename including dots
            base_name = ncm_file.stem
            f_out.write(str(ncm_file.parent / base_name) + '\n')

    print(f"Successfully generated '{input_list_path.name}' and '{output_list_path.name}' in '{output_dir}'")
    print(f"Found {len(ncm_files)} .ncm files.")

if __name__ == "__main__":
    if len(sys.argv) > 2:
        print("Usage: python find_ncm.py [directory]")
        sys.exit(1)

    target_directory = sys.argv[1] if len(sys.argv) == 2 else "."
    generate_ncm_lists(target_directory)