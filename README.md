# Scheduling Algorithms

This repository contains C++ implementations of scheduling algorithms aimed at optimizing task assignments to processors. The algorithms read data from SWF (Standard Workload Format) files, process tasks according to scheduling strategies, and output results such as task schedules and performance metrics.

## Contents

- **Algorithm Implementations**:

  - `simulated_annealing.cpp`: Implementation of the simulated annealing algorithm for task scheduling optimization.
  - `greedy_algorithm.cpp`: Implementation of a greedy algorithm based on weighted priorities.

- **Input/Output Handling**:

  - SWF file format is supported for input task definitions.
  - Results are outputted to specified files, including detailed scheduling logs and summary metrics.

## Features

- **Flexible Scheduling**:

  - Supports different scheduling strategies (e.g., based on readiness time, random permutations, and priority weights).

- **Optimization Metrics**:

  - Minimizes total completion time (`SumCj`).
  - Provides additional metrics such as `Cmax` (maximum completion time) and `Cavg` (average completion time).

- **Performance Tracking**:

  - Measures algorithm runtime and allows setting computation time limits.

## Requirements

- C++ compiler supporting C++11 or newer.

## Usage

Compile the programs using a standard C++ compiler:

```bash
g++ -std=c++11 -O2 simulated_annealing.cpp -o simulated_annealing
```

Run the program with:

```bash
./simulated_annealing <input_file.swf> <output_file.txt> <max_tasks>
```

### Example

```bash
./simulated_annealing workload.swf results.txt 500
```

## File Formats

### Input (SWF)

The input file must adhere to the SWF format:

```
; MaxProcs: 10
; MaxJobs: 500
1 0 0 30 2
2 10 0 20 3
...
```

- Lines starting with `;` contain metadata.
- Each task line includes Task ID, readiness time, placeholder (ignored), processing time, and size.

### Output

The output files detail task scheduling results, with each line:

```
Task_ID Start_Time End_Time Processor_ID(s)
```

Developed by Paweł Kierkosz and Kamil Bryła

