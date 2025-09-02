#!/usr/bin/env python3
"""Debug filename handling"""

from pathlib import Path

# Test with the problematic filename
test_file = Path("/run/media/windy/nike/Music/cloudmusic/PoppoOVO,初音ミク,重音テト - 巧克力爱意Chocolate（Feat.初音ミク,重音テト）.ncm")

print("Original filename:", test_file.name)
print("Stem (base name):", test_file.stem)
print("Extension:", test_file.suffix)
print("Parent:", test_file.parent)

# Test output path generation
output_path = test_file.parent / test_file.stem
print("Output path:", str(output_path))

# Test with Path operations
print("Path.stem preserves:", Path("PoppoOVO,初音ミク,重音テト - 巧克力爱意Chocolate（Feat.初音ミク,重音テト）.ncm").stem)