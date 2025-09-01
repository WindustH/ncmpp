
#!/bin/zsh

# Check for the correct number of arguments
if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <directory_with_ncm_files>"
  exit 1
fi

# The directory to scan, taken from the first argument
local target_dir=$1

# Check if the provided path is a directory
if [[ ! -d $target_dir ]]; then
  echo "Error: Directory not found at '$target_dir'"
  exit 1
fi

# Get the absolute path of the target directory
local abs_target_dir=$(realpath "$target_dir")

# Output file names
local input_file="ncm_input.txt"
local output_file="ncm_output.txt"

# Ensure the output files are empty before we start
> $input_file
> $output_file

# Find all .ncm files recursively and process them
# The `N` glob qualifier makes the pattern expand to nothing if no matches are found
local ncm_files=($abs_target_dir/**/*.ncm(N))

if [[ ${#ncm_files[@]} -eq 0 ]]; then
    echo "No .ncm files found."
    # Clean up empty files
    rm $input_file $output_file
    exit 0
fi

for file in $ncm_files; do
  # Write the absolute path of the ncm file to the input list
  echo "$file" >> $input_file

  # Write the base name of the file (without extension) to the output list
  # :t gets the tail (filename), :r removes the extension
  echo "$(pwd)/${file:t:r}" >> $output_file
done

echo "Successfully generated '$input_file' and '$output_file' in '$(pwd)'"
echo "Found ${#ncm_files[@]} .ncm files."
