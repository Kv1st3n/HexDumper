# Forx
**forx** is a CLI-based binary analysis tool written in C/C++17. 
Hex dump, file identification, checksum verification, and directory scanning. Built for forensics and systems work.
Developed on macOS using clang (clang-1700.4.4.1).

>[!IMPORTANT]
> This project is built and tested on macOS using clang via Xcode Command Line Tools.  
> It may work on Linux with gcc but is not officially supported.

## Prerequisites
- Xcode Command Line
- Make
- OpenSSL (`brew install openssl`)
- GTK4 & gtkmm4 (`brew install gtk4 gtkmm4`)

## Installing

```zsh
git clone https://github.com/Kv1st3n/forx.git
cd forx
make
```

## Using forx
forx reads binary data from a file and outputs it in a formatted grid, or exposes the same functionality through a GUI.

```zsh
./forx [flags] <file>
./forx [flags]          # reads from stdin if no file is given
./forx --gui            # launches the graphical interface
```

### Flags

| Flag | Description | Notes |
|------|-------------|-------------|
| `-h` | Hex dump a file or stdin | Implemented | 
| `-r` | Reverse mode — reconstruct binary from a hex dump | Implemented | 
| `-C` | Compact output (no ASCII column) | Implemented | 
| `-L` | Lowercase hex output | Implemented | 
| `-o` | Outputs a file containing the dump | Implemented | 
| `-i` | File type identifier | Implemented | 
| `-d` | Directory scanner | Implemented | 
| `-S` | String extractor | Implemented | 
| `-M` | MD5 Checksum | Implemented | 
| `-1` | SHA1 Checksum | Implemented | 
| `-2` | SHA256 Checksum | Implemented | 
| `-3` | SHA512 Checksum | Implemented |
| `-4` | SHA224 Checksum | Implemented |
| `-5` | SHA384 Checksum | Implemented |

### GUI
To access the GUI version of this tool type:
```zsh
./forx --gui
```

The GUI mirrors every CLI flag through a graphical workflow: open a file, pick a mode from the dropdown, click Run, and export the result as TXT, CSV, or PDF. It includes light/dark theming, a runtime-editable signature database path, and directory scanning support.


### Examples

```zsh
# Dump a file
./forx -h test.txt

# Read from stdin
echo "hello" | ./forx -h

# Dump a file and create a new file containing the dump
./forx -h -o dump.txt test.txt 

# Revert a dumped hex file to its original ASCII text
./forx -r -o recovered.txt dump.txt

# Use lower case flag
./forx -h -L -o  dump2.txt test.png 

# If you want to get byte size for the hexdump
./forx -h -o dump.txt -B hexDumper.c

# Compute all checksums at once
./forx -M -1 -2 -3 -4 -5 test.txt

```

## Example Output
The output is divided into the memory offset, hex values (capitalized) and printable ASCII characters.

```zsh
# Example command
./forx -h -o dump.txt hexDumper.c

# Output
00000000  23 69 6E 63 6C 75 64 65  20 3C 73 74 64 69 6F 2E  |#include <stdio.|
00000010  68 3E 0A 23 69 6E 63 6C  75 64 65 20 3C 73 74 64  |h>.#include <std|
00000020  69 6E 74 2E 68 3E 0A 23  69 6E 63 6C 75 64 65 20  |int.h>.#include |
00000030  22 68 65 78 44 75 6D 70  65 72 2E 68 22 0A 23 69  |"hexDumper.h".#i|
00000040  6E 63 6C 75 64 65 20 3C  63 74 79 70 65 2E 68 3E  |nclude <ctype.h>|
00000050  0A 23 69 6E 63 6C 75 64  65 20 3C 73 74 72 69 6E  |.#include <strin|
00000060  67 2E 68 3E 0A 0A 76 6F  69 64 20 64 75 6D 70 48  |g.h>..void dumpH|
00000070  65 78 28 46 49 4C 45 20  2A 66 69 6C 65 2C 20 46  |ex(FILE *file, F|
#continues...

# Example command with compact (-C)
./forx -h -C -o dump.txt hexDumper.c

# Output
00000000  23 69 6E 63 6C 75 64 65  20 3C 73 74 64 69 6F 2E 
00000010  68 3E 0A 23 69 6E 63 6C  75 64 65 20 3C 73 74 64 
00000020  69 6E 74 2E 68 3E 0A 23  69 6E 63 6C 75 64 65 20 
00000030  22 68 65 78 44 75 6D 70  65 72 2E 68 22 0A 23 69 
00000040  6E 63 6C 75 64 65 20 3C  63 74 79 70 65 2E 68 3E 
00000050  0A 23 69 6E 63 6C 75 64  65 20 3C 73 74 72 69 6E 
00000060  67 2E 68 3E 0A 0A 76 6F  69 64 20 64 75 6D 70 48 
00000070  65 78 28 46 49 4C 45 20  2A 66 69 6C 65 2C 20 46 
#continues...
# Will end with this if -B is included
2351 bytes


# Example command with directory scan (-d)
./forx -d /testdir -o
/testdir/test.zip: Type: ZIP

```

## Signature Database

File type signatures are loaded at runtime from `signature.txt` — no recompile needed to add new types.

## Build Commands

```zsh
# Compile
make

# Remove binary and object files
make clean
```


