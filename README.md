
# ncmpp

A fast, multi-threaded C++ tool for unlocking NCM audio files with Python helpers for complete music processing.

## Features

*   **High-Performance C++ Core:** Multi-threaded processing with optimized decryption
*   **Complete Python Toolkit:** All-in-one processing with colorful terminal output
*   **Cross-Platform:** Builds and runs on Linux and other systems with C++20 compiler
*   **Colorful Logging:** Consistent color scheme across all tools (blue paths, colored status)
*   **Smart Extension Handling:** Proper filename handling for files with dots in names
*   **Cover Art Support:** Automatic cover image embedding into converted music files
*   **Batch Processing:** Process entire directories or custom file lists

## Quick Start (Python All-in-One)

```bash
# Find and convert all .ncm files in a directory
python find_ncm.py /path/to/music

# Complete processing: convert + embed covers + cleanup
python ncmpp.py /path/to/music
```

## Components

### 1. C++ Core (`ncmpp`)
High-performance NCM file converter with multi-threading support.

### 2. Python Helpers
- **`find_ncm.py`** - Scan directories and generate file lists
- **`ncmpp.py`** - All-in-one processing pipeline
- **`embed_cover.py`** - Embed cover images into music files
- **`color_log.py`** - Shared colorful logging utility

## Dependencies

### C++ Build Dependencies
*   **OpenSSL:** Cryptographic operations
*   **RapidJSON:** Metadata parsing
*   **CMake:** Build system

### Python Dependencies
*   **mutagen:** Music file metadata handling
*   **Standard library only** - no external dependencies for basic operation

## Building (C++ Core)

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/Majjcom/ncmpp.git
    cd ncmpp
    ```

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Configure and build:**
    ```bash
    cmake ..
    cmake --build .
    ```
    The executable `ncmpp` will be created in the `build` directory.

## Usage

### Python All-in-One (Recommended)
```bash
# Complete processing pipeline
python ncmpp.py /path/to/music/directory
```

### Step-by-Step Processing
```bash
# Step 1: Find .ncm files
python find_ncm.py /path/to/music

# Step 2: Convert .ncm files using C++ tool
./build/ncmpp -i ncm_input.txt -o ncm_output.txt -s

# Step 3: Embed cover images
python embed_cover.py ncm_output.txt
```

### C++ Tool Usage
```
usage: ./ncmpp [options] ...

options:
  -h, --help            Print this message.
  -t, --threads <arg>   Max count of unlock threads. (unsigned int [=...])
  -s, --showtime        Shows how long it took to unlock everything.
  -i, --input <arg>     Path to a text file containing a list of input .ncm files. (string [=])
  -o, --output <arg>    Path to a text file containing a list of output files or a directory for fallback mode. (string [=unlocked])
```

## Examples

### Python All-in-One
```bash
# Process entire music directory
python ncmpp.py ~/Music

# Process with custom thread count
python ncmpp.py ~/Music  # Uses C++ tool internally with optimal threads
```

### C++ Tool Examples

**1. Scan and unlock current directory:**
```bash
./ncmpp
```

**2. Use 4 threads with timing:**
```bash
./ncmpp -t 4 -s
```

**3. Process file lists:**
```bash
# Custom input/output lists
./ncmpp -i input.txt -o output.txt
```

## File Structure

```
ncmpp/
├── build/           # Build directory (created during compilation)
├── ncmpp.py         # All-in-one Python processing tool
├── find_ncm.py      # Directory scanner
├── embed_cover.py   # Cover art embedder
├── color_log.py     # Shared colorful logging
├── ncmlib/          # C++ library and core tool
└── README.md
```

## License

This project is licensed under the [MIT License](LICENSE).
