# SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: MIT

#include <vector>
#include <random>
#include <iostream>
#include <chrono>
#include "DNN.h"
#include "SaveAndLoadOperations.h"

enum class OperationMode 
{
    WRITE,
    READ,
    UNKNOWN
};

static std::string FILE_NAME;
static size_t LAYERS_AMOUNT = 0;
static size_t NEURONS_PER_LAYER = 0;
static IOMode READ_MODE = IOMode::CPP;
static IOMode WRITE_MODE = IOMode::CPP;

DNN createNeuralNetwork() 
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    
    DNN network;

    // Create the layers
    for (size_t i = 0; i < LAYERS_AMOUNT; i++) 
    {
        Layer layer;

        // Create the neurons in the current layer
        for (int j = 0; j < NEURONS_PER_LAYER; j++) 
        {
            Neuron neuron;

           // Set random weights in the current neuron
            for (int k = 0; k < NEURONS_PER_LAYER; k++) 
            {
                double randomWeight = dist(gen);
                neuron.Weights.push_back(randomWeight);
            }
            
            // Set random bias in the current neuron
            neuron.Bias = dist(gen);

            layer.Neurons.emplace_back(neuron);
        }

        network.Layers.emplace_back(layer);
    }
    
    return network;
}

void printNeuralNetwork(const DNN& network)
{
    std::cout << "DNN:\n";
    std::cout << "---\n\n";

    // Iterate the layers
    for (size_t i = 0; i < network.Layers.size(); i++) 
    {
        const Layer& layer = network.Layers[i];
        std::cout << "Layer " << i + 1 << ":\n";
        
        // Iterate the neurons in the current layer
        for (size_t j = 0; j < layer.Neurons.size(); j++) 
        {
            const Neuron& neuron = layer.Neurons[j];
            std::cout << "  Neuron " << j + 1 << ": Weights = [";
            
            // Iterate the weights in the current neuron
            for (size_t k = 0; k < neuron.Weights.size(); k++) 
            {
                std::cout << neuron.Weights[k];
                if (k < neuron.Weights.size() - 1) 
                {
                    std::cout << ", ";
                }
            }
            
            std::cout << "], Bias = " << neuron.Bias << "\n";
        }
    }
} 

void printOperationTime(const std::chrono::time_point<std::chrono::high_resolution_clock>& start,
                        const std::chrono::time_point<std::chrono::high_resolution_clock>& end)
{
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "Operation time: " << duration/1000.0 << " seconds" << std::endl << std::endl;
}

void printUsageMessage(const char* argv[])
{
    std::cerr << "Usage: " << argv[0] << " <-w|-r> <filename> <arguments>" << std::endl;
    std::cerr << "  -w: Write mode - create, initialize and save a new neural network to a file" << std::endl;
    std::cerr << "      Requires the following arguments:" << std::endl;
    std::cerr << "      -l - layers amount" << std::endl;
    std::cerr << "      -n - amount of neurons per layer" << std::endl;
    std::cerr << "      -m - Write mode (cpp, posix, posix_large_chunks)" << std::endl;
    std::cerr << "  -r: Read mode - load an existing neural network from file" << std::endl;
    std::cerr << "      Requires the following arguments:" << std::endl;
    std::cerr << "      -m - Read mode (cpp, posix, posix_large_chunks)" << std::endl;
}

OperationMode parseCommandLineOptions(int argc, char* argv[])
{
    if (argc < 3)
    {
        return OperationMode::UNKNOWN;
    }

    std::string modeStr = argv[1];
    FILE_NAME = argv[2];

    if (modeStr == "-w")
    {
        if (argc != 9)
        {
            return OperationMode::UNKNOWN;
        }

        for (int i = 3; i < argc; i += 2)
        {
            std::string arg = argv[i];
            if (arg == "-l")
            {
                LAYERS_AMOUNT = static_cast<size_t>(std::stoul(argv[i + 1]));
            }
            else if (arg == "-n")
            {
                NEURONS_PER_LAYER = static_cast<size_t>(std::stoul(argv[i + 1]));
            }
            else if (arg == "-m")
            {
                WRITE_MODE = stringToIOMode(argv[i + 1]);
            }
            else
            {
                return OperationMode::UNKNOWN;
            }
        }

        if (0 < LAYERS_AMOUNT && 0 < NEURONS_PER_LAYER)
        {
            return OperationMode::WRITE;
        }
    }
    else if (modeStr == "-r") 
    {
        std::string arg = argv[3];
        if (arg == "-m")
        {
            READ_MODE = stringToIOMode(argv[4]);
            if (READ_MODE != IOMode::UNKNOWN)
            {
                return OperationMode::READ;
            }
        }
    }

    return OperationMode::UNKNOWN;
}

int main(int argc, char* argv[]) 
{   
    OperationMode mode = parseCommandLineOptions(argc, argv);

    if (mode == OperationMode::WRITE)
    {
        if (WRITE_MODE == IOMode::UNKNOWN)
        {
            printUsageMessage(const_cast<const char**>(argv));
            return 1;
        }

        // Write mode: create a new neural network and save it
        std::cout << "Creating new neural network\n";
        
        auto startTime = std::chrono::high_resolution_clock::now();
        DNN network = createNeuralNetwork();
        auto endTime = std::chrono::high_resolution_clock::now();
        printOperationTime(startTime, endTime);

        // printNeuralNetwork(network);

        std::cout << "\nSaving neural network to " << FILE_NAME << "\n";
        std::cout << "Save mode: " << IOModeToString(WRITE_MODE) << std::endl;

        startTime = std::chrono::high_resolution_clock::now();
        saveNeuralNetworkToFile(network, FILE_NAME, WRITE_MODE);
        endTime = std::chrono::high_resolution_clock::now();
        printOperationTime(startTime, endTime);
    }
    else if (mode == OperationMode::READ) 
    {
        // Read mode: load existing neural network and display it
        std::cout << "Loading neural network from: " << FILE_NAME << std::endl;
        std::cout << "Read mode: " << IOModeToString(READ_MODE) << std::endl << std::endl;
        auto startTime = std::chrono::high_resolution_clock::now();
        DNN network = readNeuralNetworkFromFile(FILE_NAME, READ_MODE);
        auto endTime = std::chrono::high_resolution_clock::now();
        
        // Check if the network was loaded successfully (has layers)
        if (network.Layers.empty()) 
        {
            std::cerr << "Failed to load neural network from " << FILE_NAME << std::endl;
            return 1;
        }
        
        // printNeuralNetwork(network);

        std::cout << "\nNeural network loaded from " << FILE_NAME << std::endl;
        printOperationTime(startTime, endTime);
    }
    else 
    {
        printUsageMessage(const_cast<const char**>(argv));
        return 1;
    }

    std::cout << std::endl;
    
    return 0;
}