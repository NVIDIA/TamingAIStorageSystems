# Hands on Lab 1
This hands on lab demonstrates the profiling and optimization process over a single compute node.

## LLM Checkpoint Save Workload
We asked an LLM (Large Language Model) to code in C++:
1. Create a fully connected DNN (Deep Neural Network) data structure.
2. Initialize the neuron's weights and biases using random numbers.
3. Store the DNN data structure to disk using Linux POSIX IO functions. 

The created program imitates a DNN checkpoint-save workload.

## Build
To build the program, execute the following:
```
cd CheckpointSave 
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make
```

## Run
To create and save a DNN data structure, specify the following command line options:
* -w - Output file path
* -l - DNN Layers amount
* -n - Amount of neurons in each layer
* -m - The write mode (cpp, posix, posix_large_chunks)

For example
```
./CheckpointSave -w /tmp/CheckPointTest/CheckPoint.bin -l 100 -n 1000 -m posix
```

To load the DNN data structure, specify the following command line options:
* -r - Input file path
* -m - The read mode (cpp, posix, posix_large_chunks)

For example
```
./CheckpointSave -r /tmp/CheckPointTest/CheckPoint.bin -m posix
```

## Performance
When creating a neural network with 100 layers, 1000 neurons per layer, the posix save time is 17.103 seconds. This is much slower than we expected.

We will use Nsight Systems to profiling this workload. CLI options are:
* profile - execute and profile a program
* 


## Profiling
For profiling this workload, we will use Nsight Systems, a free system performance analyzer.
Nsight Systems can be downloaded from https://developer.nvidia.com/nsight-systems.

To collect storage 

Show the participants how to run an nsys profile session.
Participants will be given a pre-baked nsys-rep file (Nsight Systems profiling report file), displaying a profile session that was executed on a compute node. The report contains performance data collected from CPUs, NICs, storage volumes and the operating system POSIX API.
Participants will be asked to load the nsys-rep file into Nsight Systems and inspect the time line view.
The presenter will explain the reason for the slow I/O performance: 
The LLM wrote a function that saves each neuron separately, triggering a very large amount of small IO API calls.
The presenter will show a performance improvement that saves entire layers using a single write POSIX API call.
Participants will be asked to load new nsys-rep files showing how the improved workload behaves after the fix.
