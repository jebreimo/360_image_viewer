# cppembed

This script reads a C++ file and replaces all #embed directives it finds with the contents of the corresponding file encoded as a string.

## Example:

The input file, `pdf_data.cpp.in`:

```c++
namespace foo
{
    const char* PDF_DATA[] = #embed "stairs.pdf";
}
```

is stored in the same folder as a PDF file named `stairs.pdf`.

Running the command:

```
$ cppembed pdf_data.cpp.in -o pdf_data.cpp
```

results in the output file `pdf_data.cpp`:

```c++
namespace foo
{
    const char* PDF_DATA[] = "%PDF-1.3\n%\304\345\362\345\353\247\363\240"
        "\320\304\306\n4 0 obj\n<< /Length 5 0 R /Filter /FlateDecode >>\nst"
        "ream\nx\1uUIn\33A\f\274\373\25\374@\30.\335$\373\354\27\30~\202\221"
        "\0\1\354C\342\377\3)\316\370\20MO$\b\202J\305\255X\224~\323\v\375"
        ...
        " 00000 n \ntrailer\n<< /Size 11 /Root 10 0 R /Info 1 0 R /ID [ <63b"
        "8548472f11154e3a01dcae32d2bc6>\n<63b8548472f11154e3a01dcae32d2bc6> "
        "] >>\nstartxref\n2081\n%%EOF\n";
}
```

## Encoding

**cppembed** encodes the binary data as an ASCII string using octal literals for non-characters, as this is most space-efficient and has also proven to provide the shortest compile times.

## Command line help

```
usage: cppembed.py [-h] [--stdin] [-w COLS] [-i PATH] [FILE]

positional arguments:
  FILE                  A C or C++ file with #embed directives.

optional arguments:
  -h, --help            show this help message and exit
  --stdin               Read input from stdin instead of [FILE].
  -w COLS, --width COLS
                        Set the line width. Default is 78.
  -i PATH, --include PATH
                        Add PATH to the list of paths where the program will look for the embedded files.
  -o PATH, --output PATH
                        Set the name of the output file. Default is stdout.
```
