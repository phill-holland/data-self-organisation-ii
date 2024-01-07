# Data Self Organisation-ii (Experimental)

these notes are copied from another project!  they are not relevant yet!

Project has google tests in it, only one test works! (see native.cpp)

Configured to run in vscode, cmake launch task preconfigured.

The genetic algorithm for combining programs needs to be tested next!

The parallel version of the collision detection algorithm needs to be written also next!

// ***************

This is a work in progress to investigate how a data storage solution may self organise into efficient data storage and retrieval system, with the aim of identifying data patterns of storage for abitary sentence structures (i.e. sequential data).

# Method

A three-dimensional data cube is generated, and is randomly filled with seed data, for example, we use the first verse of the song "Daisy Daisy";

```
daisy daisy give me your answer do .
I'm half crazy for the love of you .
it won't be a stylish marriage .
I can't afford a carriage .
but you'll look sweet upon the seat .
of a bicycle built for two .
```

We try to keep the dimensions of the cube as small as possible, it is imagined this will help force a more efficient solution to the problem of efficient data storage by reducing scope of the problem.

As there is commonailty between each line in the song (i.e. the same words appears more than twice) it is hoped that the system may find an optimal route to retrieve data from the cube, without having to store duplicate words within the cube (i.e. data compression).

Each co-ordinate position within the cube, has parameters of "input" and "output" gates (a set of vectors).

Data is read from the system via a given start x,y,z point within the cube, and then reads the next data value by moving onto the next co-ordinate determined by the output gate, a value is only read from a co-ordinate point if the source direction of the read is allowed by an input gates.

The system is tested to see how many complete lines from the song can be retrieved in the correct order, using the minimal amount of "jumps" possible.

# Aims

To run the program many times, and log the co-ordinates used for a successful output, to identify if there's a pattern to how the data may be organised, under the condition of a tighter and tighter data storage cube.

# Demonstrates

- Genetic Algorithms (semi pareto dominate calcuation)

- Sycl/OneApi parallel GPU code 

- Triple buffer for general population, to maximise concurrency and reduce data locking

# Todo

- Implement "output count" and add to the schema scoring system (the less steps to get to a correct, anwser the more efficient it is)

- Output from the CPU and GPU versions of the code are different (needs fixing!)

- Performance for the "pull" portion of the code get's gradually worse as the generations get higher.

- Generate graphs and cross reference outputs to confirm hypothesises

# Problems

- Genetic algorithm needs improvements, NPGA experimented with in another branch, but disappointing results (hence this may not get updated very much!)

- It get's slower and longer to find solutions the more "sentences" you configure it to look for

# Running (see requirements below)

- Project comes with VSCode pre-configured launch and build tasks

# Requirements

- Originally written on a Linux based operating system.

- This project is written in C++ and Sycl and requires that the Intel OneApi SDK is installed.

- This project will compile using Cmake and requires support for version 3.20 or above.


