# Data Self Organisation-ii (Experimental)

This application demonstrates celluar automata, genetic algorithms and paralllel programming using Sycl and Intel's OneApi toolkit.

This is a work in progress to investigate how a data storage solution may self organise into efficient data storage and retrieval system, with the aim of identifying data patterns of storage for arbitrary sentence structures (i.e. sequential data).

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

These input words are tokenised into integers and stored within an internal dictionary.

# Demonstrates

- Genetic Algorithms (semi pareto dominate calcuation)

- Cellular Automata

- Sycl/OneApi parallel GPU code 

- Triple buffer for general population, to maximise concurrency and reduce data locking

- Relative sentence scoring

# Scoring

This system implements a unique method for comparing sentences, based on their relative position to each other, rather than just by their fixed
index within a goal sentence.

For instance if we take the goal sentence of;

```
daisy daisy give me your answer do
```

and score it against the example output of;

```
your answer do bicycle carriage
```

The word chain of 'your answer do' is a partial correct answer, but however, it's positioning within the desired sentence is wrong.  The system ranks more highly the fact that 'your' is in front of 'answer' and 'do' follows it.

This method ensures it encourages partial answers, but with differing word positioning errors, for each subsequent iteration and generation, it should gradually correct those positioning errors.  This method seems to be better suited to sentence matching given this unique application of genetic algorithms.

# Todo

- Fix collision detection of native CPU code (native code has somewhat been ignored!)

- Implement different collision methods, by "age" of data in system

- Experiment with different scoring methods, one that treats the system less like a black box, and uses internal information
to score output

# Problems

- It get's slower and longer to find solutions the more "sentences" and epochs you run and configure it to look for

- It get's slower when you're looking for long output sentences (more than 5 words)

- It only needs one input word, to find a longer output sentence (encouraging input words to interact with each other whilst
in the system, for example by encouraging more word collisions makes the genetic algorithm less performant)

- May need running several times with the same parameters to find the correct answer (a common drawback of genetic algorithms)

# Potential Alternative Applications

It's conceivable that you can use this method for the encryption of messages, to generate a decryption key, the method is as followed;

- Generate a random sentence, with those words as the input dictionary of the system

- Define the message to be encrpyted as the desired output of the system

- Run the system until the output matches the random sentence generated in step one

This has several advantages, the input and output messages have no relationship with one another beyond this system that generated the "key", no information about the encrypted method is stored within the output.

The current weakness of the system is that you could potentially put any input words into the system to get an output message, this can be mitigated by encouraging more "input word" interactions with one another within the system, this can be handled by increasing this setting value, shown below;

```
parameters.scores.max_collisions = 0;
```

Increasing this number should encourage a more chaotic unpredictable behaviour of the system, the higher the number, the better it's ability to encrypt unique messages.

There are still drawbacks to this usage process though, the key in it's current form is not reusable, for each message you encrypt, you need to generate a new key, which then causes big flaws in the system, such as needing to exchange a new key each time you wish to encrypt a message, rendering the idea mostly a toy rather than having any current practical benefit.

There's an outside chance given more work this drawback can be overcome, but it may currently be technically improbable (?)

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


