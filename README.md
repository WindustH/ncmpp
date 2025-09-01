
# ncmpp

A fast, multi-threaded C++ tool for unlocking NCM audio files.

## Features

*   **Cross-Platform:** Builds and runs on Linux and other systems with a C++20 compiler.
*   **Multi-threaded:** Utilizes multiple threads to process files in parallel for maximum speed.
*   **Flexible Input:**
    *   Process individual `.ncm` files.
    *   Scan a directory for all `.ncm` files.
    *   Read a list of input files from a text file.
*   **Flexible Output:**
    *   Specify an output directory.
    *   Provide a list of output file paths for 1-to-1 mapping.

## Dependencies

*   **OpenSSL:** Required for the cryptographic operations.
*   **RapidJSON:** Required for parsing metadata.
*   **CMake:** Required for building the project.

## Building

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

3.  **Configure the project with CMake:**
    ```bash
    cmake ..
    ```

4.  **Build the project:**
    ```bash
    cmake --build .
    ```
    The executable `ncmpp` will be created in the `build` directory.

## Usage

```
usage: ./ncmpp [options] ... 

options:
  -h, --help            Print this message.
  -t, --threads <arg>   Max count of unlock threads. (unsigned int [=...])
  -s, --showtime        Shows how long it took to unlock everything.
  -i, --input <arg>     Path to a text file containing a list of input .ncm files. (string [=])
  -o, --output <arg>    Path to a text file containing a list of output files or a directory for fallback mode. (string [=unlocked])
```

### Examples

**1. Scan the current directory and unlock files to the `unlocked` directory:**
```bash
./ncmpp
```

**2. Use 4 threads and show the time taken:**
```bash
./ncmpp -t 4 -s
```

**3. Process a list of input files and save to a specific directory:**
*   `input.txt`:
    ```
    /path/to/song1.ncm
    /path/to/song2.ncm
    ```
*   Command:
    ```bash
    ./ncmpp -i input.txt -o /path/to/unlocked_music
    ```

**4. Process a list of input files with a 1-to-1 output mapping:**
*   `input.txt`:
    ```
    /path/to/song1.ncm
    /path/to/song2.ncm
    ```
*   `output.txt`:
    ```
    /path/to/unlocked/song1.mp3
    /path/to/unlocked/song2.flac
    ```
*   Command:
    ```bash
    ./ncmpp -i input.txt -o output.txt
    ```

## License

This project is licensed under the [MIT License](LICENSE).
