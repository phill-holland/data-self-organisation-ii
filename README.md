# Data Self Organisation-ii (Experimental)

This application demonstrates celluar automata, genetic algorithms and paralllel programming using Sycl and Intel's OneApi toolkit.

This is a work in progress to investigate how a data storage solution may self organise into efficient data storage and retrieval system, with the aim of identifying data patterns of storage for abitary sentence structures (i.e. sequential data).

The underlying principal is to test if a fundamentally chaotic system can self-organise into a stable data storage and retrieval system.

# Method

Data is given a 3D dimension cube in order to move around in, one grid cell at a time.  You can insert sentences into the system, using varyingly different randomly generated patterns, and pre-load the 3D cube with static data before the main loop starts.

Data is configured to behave in different ways when they collide with one another, potentially changing it's path of direction.

Output is determined when one data cell collides with another static data cell.

For this test program, we are using the song Daisy Daisy as a small data input test sample.

```
daisy daisy give me your answer do .
I'm half crazy for the love of you .
it won't be a stylish marriage .
I can't afford a carriage .
but you'll look sweet upon the seat .
of a bicycle built for two .
```

# Demonstrates

- Genetic Algorithms (semi pareto dominate calcuation)

- Cellular Automata

- Sycl/OneApi parallel GPU code 

- Triple buffer for general population, to maximise concurrency and reduce data locking

# Todo

- Implement high resolution clock for measuring frame rate of system

- Fix collision detection of native CPU code

- Implement different collision methods, by "age" of data in system

- Implement different collision methods, by "data" in the system (words determine direction of data movemen)

# Problems

- It get's slower and longer to find solutions the more "sentences" and epochs you run and configure it to look for

# Building Running (see requirements below)

- Project comes with VSCode pre-configured launch and build tasks, however to manually build;

```
source /opt/intel/oneapi/setvars.sh --include-intel-llvm --force
cd build
cmake -DCMAKE_CXX_FLAGS="-O0 -g" -DBUILD_FOR_CUDA=true ../
cmake --build .
```

Assuming Intel OneApi is pre-installed and you have a newish Nvidia GPU (it will probably compile and run on GTX 10xx series and above)

Running the tests can be accomplished by;

```
cd build
ctest
```

The build process should pop an executable file in the build directory to run the application, it is advised to
run the system from within VSCode, to tweak the input parameters!

# Requirements

- Originally written on a Linux based operating system.

- This project is written in C++ and Sycl and requires that the Intel OneApi SDK is installed.

- This project will compile using Cmake and requires support for version 3.20 or above.

- Nvidia's Cuda must also be installed, in order for you to target Nvidia GPUs.


