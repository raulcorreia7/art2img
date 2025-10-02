# art2img - Extract Images from Duke Nukem 3D Art Files

A simple tool to convert Duke Nukem 3D art files into modern PNG or TGA images.

## What's New
- **Version**: 1.0.0
- **Version support**: Added the ability to check the version with `--version` or `-v`
- **Transparency**: Added the ability to automatically handle magenta transparency

## Quick Start
- Download the latest release from the [GitHub releases](https://github.com/raulcorreia7/art2img/releases)
- Extract the art files with the art2img command
- Run the command to check the version with the command `--version`

## Example
- Basic usage:
  ```bash
  # Extract a single file
  ./bin/art2img TILES000.ART
  ```

- Directory with merged files:
  ```bash
  # Extract a directory of files
  ./bin/art2img -m -o /output /path
  ```

## Output Formats
- Output formats in PNG and TGA
  - PNG output with RGBA support
  - PNG output with proper alpha channel
  - Automatic transparency for magenta pixels
  - Automatic output with premultiplied alpha

## Command Line Options
- `--version` or `-v`  show version information
- `--format` or `-f` to specify the output format
- `--output` or `-o` to specify the output directory
- `-p` for palette file
- `-n` for no animation
- `-m` for merging animation data
- `-t` for specifying threads
- `-q` for quiet output

## Building
- Build with the build system:
  ```bash
  make
  ```

## Requirements
- Build requires:
  - C++17 compiler
  - POSIX threads
  - Make
  - Linux
- Build for:
  - Windows build
  - Linux build
  - Cross-compile build
- Run:
  - No dependencies
  - Docker