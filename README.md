# LOP Project – Build & Run Instructions

---

# 1. Compiling the Project

## 1. Windows 11

### Option 1: GCC (MinGW-w64 via MSYS2)

#### Step 1: Install MSYS2

Download and install MSYS2 from: [https://www.msys2.org](https://www.msys2.org)

#### Step 2: Install GCC

Open the MSYS2 app/terminal and run:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
```

#### Step 3: Add GCC to PATH

Add the following directory to your system PATH: `C:\msys64\ucrt64\bin`

Instructions:

- In Windows Search, type "system variables" and select "Edit the system environment variables."
- Click "Environment Variables…". In the System variables section, scroll down and select "Path."
- Make sure you edit the existing Path variable — creating a new one will not work. Windows must update the existing PATH to find the command.
- Click "Edit" → "New", then add: `C:\msys64\ucrt64\bin`.
- Click "OK" on all windows to save and close.

#### Step 4: Verify Installation

Open a new Command Prompt and run:

```bash
gcc --version
```

#### Step 5: Compile the Project

Run the following command inside the `./LOP/LOP` directory, where all the C source files referenced in the command are located:

```bash
gcc instance.c optimization.c timer.c utilities.c algorithms.c cw.c vnd.c main_teacher.c main_lop.c main_vnd.c main.c -o LOP
```

After the build is complete, the executable (`LOP.exe`) will be located in the same `./LOP/LOP` directory where you ran the gcc command.

---

### Option 2: Visual Studio

#### Step 1: Install Visual Studio

Download and install Visual Studio from: [https://visualstudio.microsoft.com/downloads/](https://visualstudio.microsoft.com/downloads/)

#### Step 2: Open Project

Open the `LOP.slnx` project file in Visual Studio. (You can find it in the root LOP folder.)

#### Step 3: Build Project

In the top menu, click "Build", then select "Rebuild Solution" from the dropdown.

After the build is complete, the executable (`LOP.exe`) will be generated in the `./LOP/x64/Debug/` directory.

---

## 2. macOS

To compile C code on macOS using GCC (Apple Clang):

1. Open a Terminal and install the Xcode Command Line Tools by running:
   ```bash
   xcode-select --install
   ```
2. Follow the popup instructions to complete the installation.
3. Verify the installation by opening a new Terminal and running:
   ```bash
   gcc --version
   ```
   You should see output like: "Apple clang version ...".

Run the following command inside the `./LOP/LOP` directory (on macOS, the top-level folder may be `LOP-main` depending on how the project was extracted), where all the C source files referenced in the command are located:

```bash
gcc instance.c optimization.c timer.c utilities.c algorithms.c cw.c vnd.c main_teacher.c main_lop.c main_vnd.c main.c -o LOP
```

After the build completes, the executable (`LOP`, a Unix executable file) will be located in the same `./LOP/LOP` (or `./LOP-main/LOP`) directory where you ran the gcc command.

---

## 3. Linux

To compile C code on Linux, you need to install GCC (GNU Compiler Collection).

1. Install GCC. Depending on your distribution, use one of the following commands:

   - **Ubuntu / Debian:**
     ```bash
     sudo apt update
     sudo apt install build-essential
     ```
   - **Fedora:**
     ```bash
     sudo dnf install gcc
     ```
   - **Arch Linux:**
     ```bash
     sudo pacman -S base-devel
     ```

2. Verify the installation by opening a new Terminal and running:
   ```bash
   gcc --version
   ```

Run the following command inside the `./LOP/LOP` directory, where all the C source files referenced in the command are located:

```bash
gcc instance.c optimization.c timer.c utilities.c algorithms.c cw.c vnd.c main_teacher.c main_lop.c main_vnd.c main.c -o LOP
```

After the build completes, the executable (`LOP`) will be located in the same `./LOP/LOP` directory where you ran the gcc command.

---

# 2. Running the Project

## 1. Run a Single Instance

### Command Format

```bash
./LOP(.exe) -m <lop|vnd> -i <input_file> [-b <best_known_value>]
```

### Notes

- `-m <lop|vnd>`: Selects which algorithm to run
  - `lop`: Runs iterative improvement using all 12 combinations of starting solution (Random, CW), neighbourhood (Transpose, Exchange, Insert), and pivot rule (First-improvement, Best-improvement).
  - `vnd`: Runs Variable Neighbourhood Descent using the CW starting solution, comparing two neighbourhood orderings (Transpose→Exchange→Insert and Transpose→Insert→Exchange).
- Replace `<input_file>` with the path to your instance file.
- Replace `<value>` with the most optimal known cost for that instance.
  - This is optional but allows the program to calculate deviation.
  - The optimal costs can be found in `./LOP/data/best_known/best_known.txt`.
- The command must be executed from the directory containing `LOP.exe` (see above).

Open a command prompt, navigate to the directory containing `LOP.exe`, and run the command.

### Example (Windows – Command Prompt)

```bash
C:\LOP\x64\Debug> ./LOP.exe -m lop -i ..\..\data\instances\N-be75eec_150 -b 3482828
```

### Linux / macOS

```bash
\LOP\LOP> ./LOP -m lop-i ..\data\instances\N-be75eec_150 -b 3482828
```

> **Warning:** If you compile the project using Visual Studio, the generated `.exe` file will be located in `./LOP/x64/Debug/`. This differs from compiling with gcc (on any operating system), where the executable is placed in `./LOP/LOP/`. Because of this difference, some of the file paths used in the Windows examples below may need to be adjusted when using gcc. For example, instead of:
> ```
> ..\..\data\instances   (Visual Studio)
> ```
> you should use:
> ```
> ..\data\instances     (gcc)
> ```
> Also note that you cannot blindly copy the commands as-is. You still need to replace the placeholder `-m <lop|vnd>` with the appropriate option, depending on whether you want to use `lop` or `vnd`.

---

## 2. Run All Instance Files

### Windows (PowerShell)

```powershell
Get-ChildItem "..\..\data\instances" | ForEach-Object {
    ./LOP.exe -m <lop|vnd> -i $_.FullName
}
```

### Linux / macOS

```bash
for f in ../data/instances/*; do ./LOP -m <lop|vnd> -i "$f"; done
```

---

## 3. Run All Instances with Best Known Values

### Windows (PowerShell)

```powershell
$best = @{}
Get-Content "..\..\data\best_known\best_known.txt" | ForEach-Object {
    $parts = $_ -split "\s+"
    if ($parts.Count -ge 2) {
        $best[$parts[0]] = $parts[1]
    }
}

Get-ChildItem "..\..\data\instances" -File | ForEach-Object {
    $b = $best[$_.Name]
    if ($b) {
        ./LOP.exe -m <lop|vnd> -i $_.FullName -b $b
    } else {
        ./LOP.exe -m <lop|vnd> -i $_.FullName
    }
}
```

### Linux / macOS

```bash
for f in ../data/instances/*; do
    name=$(basename "$f")
    b=$(awk -v n="$name" '$1 == n {print $2}' ../data/best_known/best_known.txt)
    if [ -n "$b" ]; then
        ./LOP -m <lop|vnd> -i "$f" -b "$b"
    else
        ./LOP -m <lop|vnd> -i "$f"
       fi
   done
```

---

# 3. Project Structure Overview

In addition to the executable file located in `./LOP/x64/Debug/`, the project includes two important result directories:

## 1. implementation_1_results

This folder contains all files related to Exercise 1, which focuses on basic iterative improvement search algorithms.

**Contents:**

- **Instance result files (.csv)**
  For each instance problem provided in `./LOPdata/instances`, a CSV file is generated. These files follow the naming convention: `<instance-name>-result.csv` (for example: `N-tiw56r72_250-result.csv`). Each file contains 12 results, corresponding to all combinations of:
  - Pivot rules: first improvement, best improvement
  - Neighborhoods: insert, exchange, transpose
  - Initial configurations: uninformed random picking, Chenery and Watanabe (CW)

- **combine.py**
  A Python script that reads all CSV result files in the current working directory and combines them into a single text file.

- **combined-results.txt**
  The aggregated results produced by running `combine.py`.

- **analysis_1.txt**
  Contains the output of statistical tests performed in R. This includes the average percentage deviation from the best known solution, the total execution time across all instances per algorithm setup, pairwise Student's t-test p-values (paired, two-sided) comparing all algorithm combinations, and Wilcoxon signed-rank test results identifying pairs with statistically significant performance differences (p < 0.05) along with which algorithm performed better in each case.

- **deviation_boxplot_1.pdf**
  A visualization generated in R, showing boxplots of the deviation percentage for each algorithm setup.

---

## 2. implementation_2_results

This folder contains all files related to Exercise 2, which focuses on the Variable Neighborhood Descent (VND) algorithm.

**Contents:**

- **Instance result files (.csv)**
  For each instance problem in `./LOPdata/instances`, a CSV file is generated. These files follow the same naming convention: `<instance-name>-result.csv`. Each file contains 2 results, corresponding to the two possible orderings of neighborhood structures:
  - transpose → exchange → insert
  - transpose → insert → exchange

- **combine.py**
  A Python script that reads all CSV result files in the current working directory and combines them into a single text file.

- **combined-results.txt**
  The aggregated results produced by running `combine.py`.

- **analysis_2.txt**
  Contains the output of statistical tests performed in R. This includes the average percentage deviation from the best known solution, the total execution time across all instances per algorithm setup, pairwise Student's t-test p-values (paired, two-sided) comparing all algorithm combinations, and Wilcoxon signed-rank test results identifying pairs with statistically significant performance differences (p < 0.05) along with which algorithm performed better in each case.

- **deviation_boxplot_2.pdf**
  A visualization generated in R, showing boxplots of the deviation percentage for each algorithm setup.

---

## Source Code Location

The main source code is organized within a Visual Studio solution (`.slnx`). Since the project is named LOP, all source files are located in `./LOP/LOP/`. On macOS, this may instead be `./LOP-main/LOP/`.

This directory contains:

- All C source files (`.c`)
- All C header files (`.h`)
- All R scripts used for statistical testing

---

## README

The `README.md` file is located in the root of the project directory: `./LOP/`
