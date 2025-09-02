#!/usr/bin/env python3
"""
embed_cover.py - Embed cover images into music files and delete the images

Usage: python embed_cover.py filepath_with_no_ext_to_deal.txt

This script reads a file containing paths without extensions, finds corresponding
.jpg cover images and .flac/.mp3 music files, embeds the cover into the music file,
and then deletes the cover image.
"""

import sys
import os
from pathlib import Path
import mutagen
from mutagen.flac import FLAC, Picture
from mutagen.mp3 import MP3
from mutagen.id3 import ID3, APIC
import mimetypes


def embed_cover_into_music(music_file_path, cover_file_path):
    """Embed cover image into music file."""
    try:
        file_ext = music_file_path.suffix.lower()

        if file_ext == '.flac':
            return embed_cover_into_flac(music_file_path, cover_file_path)
        elif file_ext == '.mp3':
            return embed_cover_into_mp3(music_file_path, cover_file_path)
        else:
            print(f"Unsupported file format: {file_ext}")
            return False

    except Exception as e:
        print(f"Error embedding cover into {music_file_path}: {e}")
        return False


def embed_cover_into_flac(music_file_path, cover_file_path):
    """Embed cover image into FLAC file."""
    try:
        audio = FLAC(music_file_path)

        # Read the cover image
        with open(cover_file_path, 'rb') as f:
            cover_data = f.read()

        # Create a Picture object
        picture = Picture()
        picture.type = 3  # Front cover
        picture.desc = 'Front Cover'
        picture.mime = mimetypes.guess_type(str(cover_file_path))[0] or 'image/jpeg'
        picture.data = cover_data

        # Remove existing pictures
        audio.clear_pictures()

        # Add the new cover
        audio.add_picture(picture)
        audio.save()

        print(f"Successfully embedded cover into FLAC: {music_file_path.name}")
        return True

    except Exception as e:
        print(f"Error embedding cover into FLAC: {e}")
        return False


def embed_cover_into_mp3(music_file_path, cover_file_path):
    """Embed cover image into MP3 file."""
    try:
        audio = MP3(music_file_path, ID3=ID3)

        # Read the cover image
        with open(cover_file_path, 'rb') as f:
            cover_data = f.read()

        # Determine MIME type
        mime_type = mimetypes.guess_type(str(cover_file_path))[0] or 'image/jpeg'

        # Remove existing APIC frames
        audio.tags.delall('APIC')

        # Add the new cover
        audio.tags.add(
            APIC(
                encoding=3,  # UTF-8
                mime=mime_type,
                type=3,  # Front cover
                desc='Front Cover',
                data=cover_data
            )
        )
        audio.save()

        print(f"Successfully embedded cover into MP3: {music_file_path.name}")
        return True

    except Exception as e:
        print(f"Error embedding cover into MP3: {e}")
        return False


def process_file(base_path):
    """Process a single base path (without extension)."""
    base_path = Path(base_path.strip())

    # Look for cover image (.jpg)
    cover_file = base_path.with_suffix('.jpg')
    if not cover_file.exists():
        print(f"Cover image not found: {cover_file}")
        return False

    # Look for music files (.flac or .mp3)
    music_files = []
    for ext in ['.flac', '.mp3']:
        music_file = base_path.with_suffix(ext)
        if music_file.exists():
            music_files.append(music_file)

    if not music_files:
        print(f"No music files found for: {base_path}")
        return False

    success_count = 0
    for music_file in music_files:
        print(f"Processing: {music_file.name}")

        if embed_cover_into_music(music_file, cover_file):
            success_count += 1

    # Delete the cover image if at least one embedding was successful
    if success_count > 0:
        try:
            cover_file.unlink()
            print(f"Deleted cover image: {cover_file.name}")
        except Exception as e:
            print(f"Warning: Could not delete cover image: {e}")

    return success_count > 0


def main():
    """Main function."""
    if len(sys.argv) != 2:
        print("Usage: python embed_cover.py filepath_with_no_ext_to_deal.txt")
        sys.exit(1)

    input_file = Path(sys.argv[1])

    if not input_file.exists():
        print(f"Error: Input file not found: {input_file}")
        sys.exit(1)

    print(f"Processing file: {input_file}")

    success_count = 0
    failure_count = 0

    with open(input_file, 'r', encoding='utf-8') as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            if not line:  # Skip empty lines
                continue

            print(f"\nProcessing line {line_num}: {line}")

            if process_file(line):
                success_count += 1
            else:
                failure_count += 1

    print(f"\nProcessing complete!")
    print(f"Successfully processed: {success_count} files")
    print(f"Failed to process: {failure_count} files")


if __name__ == "__main__":
    main()