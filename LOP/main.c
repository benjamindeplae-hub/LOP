/*
 * To run C code with GCC on Windows, you need to install MinGW-w64 (Minimalist GNU for Windows), which includes GCC.
 * 1. Go to msys2.org and download the installer
 * 2. Run the installer and follow the setup steps
 * 3. Open the MSYS2 terminal and run:
 *    $ pacman -S mingw-w64-ucrt-x86_64-gcc
 * 4. Add GCC to your system PATH: C:\msys64\ucrt64\bin
 * Then verify if it works by opening Command Prompt and typing:
 *    $ gcc --version
 *
 * You need to edit the existing Path variable instead. A new variable won't help Windows find the command.
 * Under "System variables", scroll down and find the existing Path variable
 * Click on Path -> click "Edit" -> Click "New" and paste: "C:\msys64\ucrt64\bin"
 * Then you can open a fresh Command Prompt and run gcc --version.
 */

/*
 * To compile the project, use the following command:
 *   $ gcc instance.c optimization.c timer.c utilities.c algorithms.c cw.c vnd.c main_teacher.c main_lop.c main_vnd.c main.c -o LOP
 *
 * The above GCC command silently fails with a linkage error; I couldn’t find a fix.
 * However, you can build the solution using Visual Studio instead.
 * This will create a .exe file in the folder ./LOP/x64/Debug.
 * For example:
 * C:\Users\user\vub\master_ai\semester_2\heuristic\task\LOP\x64\Debug
 *
 *
 * To run the project, use the following command format:
 * ./LOP.exe -m <lop|vnd> -i <input_file> [-b <best_known_value>]
 *
 * Notes:
 * - m <lop|vnd>: Selects which algorithm to run
 *      - lop: Runs iterative improvement using all 12 combinations of starting solution (Random, CW), neighbourhood (Transpose, Exchange, Insert), and pivot rule (First-improvement, Best-improvement).
 *      - vnd: Runs Variable Neighbourhood Descent using the CW starting solution, comparing two neighbourhood orderings (Transpose→Exchange→Insert and Transpose→Insert→Exchange).
 * - Replace <input_file> with the path to your instance file.
 * - Replace <value> with the most optimal known cost for that instance.
 *      - This is optional but allows the program to calculate deviation.
 *      - The optimal costs can be found in ./LOP/data/best_known/best_known.txt.
 * - The command must be executed from the directory where the executable is located (see above).
 * - Open a command prompt, navigate to the directory containing LOP.exe, and run the command from there.
 *
 * Example (Windows CMD):
 * C:\LOP\x64\Debug> ./LOP.exe -m lop -i ..\..\data\instances\N-be75eec_150 -b 3482828
 *
 * Linux / macOS:
 * ./LOP -m lop -i ../../data/instances/N-be75eec_150 -b 3482828
 *
 *
 * To instantly run for all instance files in the instances folder:
 * Windows (PowerShell):
   $ Get-ChildItem "..\..\data\instances" | ForEach-Object {
        ./LOP.exe -m <lop|vnd> -i $_.FullName
     }
 *
 * Linux/Mac:
   $ for f in ../../data/instances/*; do ./LOP.exe -m <lop|vnd> -i "$f"; done
 *
 *
 * To instantly run for all instance files in the instances folder with the associated optimal cost:
 * Windows (PowerShell):
 * $ $best = @{}
   Get-Content "..\..\data\best_known\best_known.txt" | ForEach-Object {
       $parts = $_ -split "\s+"
       if ($parts.Count -ge 2) { $best[$parts[0]] = $parts[1] }
   }
   Get-ChildItem "..\..\data\instances" -File | ForEach-Object {
       $b = $best[$_.Name]
       if ($b) { ./LOP.exe -m <lop|vnd> -i $_.FullName -b $b }
       else     { ./LOP.exe -m <lop|vnd> -i $_.FullName }
   }
 *
 * Linux/Mac:
   $ for f in ../../data/instances/*; do
       name=$(basename "$f")
       b=$(awk -v n="$name" '$1 == n {print $2}' ../../data/best_known/best_known.txt)
       if [ -n "$b" ]; then
           ./LOP.exe -m <lop|vnd> -i "$f" -b "$b"
       else
           ./LOP.exe -m <lop|vnd> -i "$f"
       fi
   done
 */

#include <stdio.h>
#include <string.h>

// Forward declarations
int main_lop(int argc, char** argv);
int main_vnd(int argc, char** argv);

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s -m <lop|vnd> -i <file> [-b <value>]\n", argv[0]);
        return 1;
    }

    // Find and consume the -m flag, build a new argv without it
    char* mode = NULL;
    char* filtered[64];
    int filteredCount = 0;
    filtered[filteredCount++] = argv[0];

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            mode = argv[++i];  // consume both -m and its value
        }
        else {
            filtered[filteredCount++] = argv[i];
        }
    }

    if (mode == NULL) {
        printf("Error: -m <lop|vnd> is required.\n");
        printf("Usage: %s -m <lop|vnd> -i <file> [-b <value>]\n", argv[0]);
        return 1;
    }

    if (strcmp(mode, "lop") == 0) {
        return main_lop(filteredCount, filtered);
    }
    else if (strcmp(mode, "vnd") == 0) {
        return main_vnd(filteredCount, filtered);
    }
    else {
        printf("Unknown mode: %s. Use 'lop' or 'vnd'.\n", mode);
        return 1;
    }
}