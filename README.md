# LOP Project – Build & Run Instructions

This document explains how to compile and run the LOP project on Windows, Linux, and macOS.

---

# 1. Compiling the Project

## Option 1: GCC (MinGW-w64 via MSYS2)

### Step 1: Install MSYS2

Download and install MSYS2 from: [https://www.msys2.org](https://www.msys2.org)

### Step 2: Install GCC

Open the MSYS2 terminal and run:

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc
```

### Step 3: Add GCC to PATH

Add the following directory to your system PATH:

```
C:\msys64\ucrt64\bin
```

Instructions:

* Open **System Variables**
* Find and select `Path`
* Click **Edit**
* Click **New** and paste the path above

> Important: Editing the existing Path variable is required. Creating a new variable will NOT work.

### Step 4: Verify Installation

Open Command Prompt and run:

```bash
gcc --version
```

---

### Step 5: Compile the Project

Run the following command:

```bash
gcc instance.c optimization.c timer.c utilities.c algorithms.c cw.c vnd.c main_teacher.c main_lop.c main_vnd.c main.c -o LOP
```

---

## Option 2: Visual Studio

### Step 1: Install Visual Studio

Download and install from: [https://visualstudio.microsoft.com/downloads/](https://visualstudio.microsoft.com/downloads/)

### Step 2: Open Project

Open the solution file:

```
LOP.slnx
```

(found in the root LOP folder)

### Step 3: Build Project

In the top menu:

```
Build → Rebuild Solution
```

### Output

The executable will be generated at:

```
./LOP/x64/Debug/
```

---

# 2. Running the Project

## Run a Single Instance

### Command Format

```bash
./LOP.exe -m <lop|vnd> -i <input_file> [-b <best_known_value>]
```

### Parameters

* `-m <lop|vnd>`: Selects which algorithm to run
  * `lop`: Runs iterative improvement using all 12 combinations of starting solution (Random, CW), neighbourhood (Transpose, Exchange, Insert), and pivot rule (First-improvement, Best-improvement)
  * `vnd`: Runs Variable Neighbourhood Descent using the CW starting solution, comparing two neighbourhood orderings (Transpose→Exchange→Insert and Transpose→Insert→Exchange)
* `<input_file>`: Path to the instance file
* `<best_known_value>` (optional): Known optimal cost
  * Used to compute deviation
  * Available in:

```
./LOP/data/best_known/best_known.txt
```

### Example (Windows CMD)

```bash
C:\LOP\x64\Debug> ./LOP.exe -m lop -i ..\..\data\instances\N-be75eec_150 -b 3482828
```

### Linux / macOS

```bash
./LOP -m lop -i ../../data/instances/N-be75eec_150 -b 3482828
```

---

## Run All Instance Files

### Windows (PowerShell)

```powershell
Get-ChildItem "..\..\data\instances" | ForEach-Object {
    ./LOP.exe -m <lop|vnd> -i $_.FullName
}
```

### Linux / macOS

```bash
for f in ../../data/instances/*; do ./LOP.exe -m <lop|vnd> -i "$f"; done
```

---

## Run All Instances with Best Known Values

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
for f in ../../data/instances/*; do
    name=$(basename "$f")
    best=$(grep "$name" ../../data/best_known/best_known.txt | awk '{print $2}')

    if [ -n "$best" ]; then
        ./LOP.exe -m <lop|vnd> -i "$f" -b "$best"
    else
        ./LOP.exe -m <lop|vnd> -i "$f"
    fi
done
```

---

# Notes

* Always run commands from the directory containing `LOP.exe`
* Ensure paths are correct relative to your execution location
* Windows users should prefer PowerShell for batch execution

---

# 3. Project Structure Overview:

In addition to the executable file located in ./LOP/x64/Debug/, the project includes two important result directories:

## 1. implementation_1_results:

This folder contains all files related to Exercise 1, which focuses on basic iterative improvement search algorithms.

Contents:

- Instance result files (.csv)
For each instance problem provided in ./LOPdata/instances, a CSV file is generated.
These files follow the naming convention: <instance-name>-result.csv
For example: N-tiw56r72_250-result.csv
Each file contains 12 results, corresponding to all combinations of:
o	Pivot rules: first improvement, best improvement 
o	Neighborhoods: insert, exchange, transpose 
o	Initial configurations: uninformed random picking, Chenery and Watanabe (CW)

- combine.py
A Python script that reads all CSV result files in the current working directory and combines them into a single text file.

- combined-results.txt
The aggregated results produced by running combine.py.

- analysis_1.txt
Contains the output of statistical tests performed in R. This includes the average percentage deviation from the best known solution, the total execution time across all instances per algorithm setup, pairwise Student's t-test p-values (paired, two-sided) comparing all algorithm combinations, and Wilcoxon signed-rank test results identifying pairs with statistically significant performance differences (p < 0.05) along with which algorithm performed better in each case.

- deviation_boxplot_1.pdf
A visualization generated in R, showing boxplots of the deviation percentage for each algorithm setup.

---

## 2. implementation_2_results:

This folder contains all files related to Exercise 2, which focuses on the Variable Neighborhood Descent (VND) algorithm.

Contents:

- Instance result files (.csv)
For each instance problem in ./LOPdata/instances, a CSV file is generated.
These files follow the same naming convention: <instance-name>-result.csv
Each file contains 2 results, corresponding to the two possible orderings of neighborhood structures:
o	transpose → exchange → insert 
o	transpose → insert → exchange

- combine.py
A Python script that reads all CSV result files in the current working directory and combines them into a single text file.

- combined-results.txt
The aggregated results produced by running combine.py.combined-results.txt with the combined results procuced by the python combine.py file

- analysis_2.txt
Contains the output of statistical tests performed in R. This includes the average percentage deviation from the best known solution, the total execution time across all instances per algorithm setup, pairwise Student's t-test p-values (paired, two-sided) comparing all algorithm combinations, and Wilcoxon signed-rank test results identifying pairs with statistically significant performance differences (p < 0.05) along with which algorithm performed better in each case.

- deviation_boxplot_2.pdf
A visualization generated in R, showing boxplots of the deviation percentage for each algorithm setup.

---

## Source Code Location:

The main source code is organized within a Visual Studio solution (.slnx).
Since the project is named LOP, all source files are located in: ./LOP/LOP/
This directory contains:
- All C source files (.c) 
- All C header files (.h) 
- All R scripts used for statistical testing
