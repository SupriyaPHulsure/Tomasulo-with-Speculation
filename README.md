
# Tomasulo-Simulator with hardware Speculation
Tomasulo’s algorithm is a computer architecture hardware algorithm for dynamic scheduling of instructions that allows out-of-order execution and enables more efficient use of multiple execution units. It was developed by Robert Tomasulo at IBM in 1967 and was first implemented in the IBM System/360 Model 91’s floating point unit.

This project is a simulator for an out-of-order superscalar processor that uses the Tomasulo algorithm and fetches NF instructions per cycle. We design the simulator to maintain consistent state in the presence of exceptions by using hardware speculation which uses two hardware components: 1. ROB and 2. Renaming Resgisters. We also use the simulator to find the appropriate number of function units, fetch rate and result buses for given benchmark.

### Getting Started
This project is implemented in C language. 


### Prerequisites

You need C compiler to excute this project.

 

### Running the tests

I. Confirguaration details
 
    // Configuration details
    NF (Number of instructions that can be fetched in each cycle) = 4;
    NI (Number of Instruction that can be issued in each cycle)= 8;
    NR (Number of instruction in ROB - Reorder Buffer) = 16;
    NB (Number of instruction on Common data bus) = 4;

    
II. Run
```
  >cd TomasuloSimulator

  >make clean

  >make
  ```

  For single instruction file (single thread):
```
  >./TomasuloSimulator Benchmarks/Test1.dat
```
  For multithreading using two different instruction files:
```
  >./TomasuloSimulator Benchmarks/Test1.dat Benchmarks/Test1.dat
```
  In order to change configuration files:
```
  >vim Config/TomasuloSimulator.conf
```
