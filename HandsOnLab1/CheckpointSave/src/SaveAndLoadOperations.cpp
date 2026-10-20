// SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT

#include <fcntl.h>
#include <unistd.h>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <nvtx3/nvtx3.hpp>
#include "SaveAndLoadOperations.h"

std::string IOModeToString(const IOMode& mode)
{
    switch (mode)
    {
        case IOMode::CPP:
            return "cpp";
        case IOMode::POSIX:
            return "posix";
        case IOMode::POSIX_LARGE_CHUNKS:
            return "posix_large_chunks";
        default:
            return "unknown";
    }
}

IOMode stringToIOMode(const std::string& modeStr)
{
    if (modeStr == "cpp")
        return IOMode::CPP;
    else if (modeStr == "posix")
        return IOMode::POSIX;
    else if (modeStr == "posix_large_chunks")
        return IOMode::POSIX_LARGE_CHUNKS;
    else
        return IOMode::UNKNOWN;
}

// Set file access pattern to minimize caching effects
void setNoCachedFile(int fd, const std::string& filename)
{
    int rc = posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
    if (rc != 0)
    {
        std::cerr << "Failed setting advice on: " << filename << std::endl;
    }
}

// Save a neural network to a file using C++ standard library
void saveNeuralNetworkToFileCpp(const DNN& network, const std::string& filename)
{
    std::ofstream file(filename, std::ios::trunc | std::ios::binary);
    if (!file)
    {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    // Output the network's sizes
    uint64_t layersCount = network.Layers.size();
    uint64_t neuronsPerLayer = network.Layers[0].Neurons.size();
    file.write(reinterpret_cast<const char*>(&layersCount), sizeof(layersCount));
    file.write(reinterpret_cast<const char*>(&neuronsPerLayer), sizeof(neuronsPerLayer));

    uint32_t layerIndex = 0;
    for (const Layer& layer : network.Layers) 
    {
        std::string layerTag = "Saving layer " + std::to_string(layerIndex++);
        nvtx3::scoped_range layerRange(layerTag.c_str());

        for (const Neuron& neuron : layer.Neurons) 
        {
            for (double weight : neuron.Weights) 
            {
                file.write(reinterpret_cast<const char*>(&weight), sizeof(weight));
            }

            file.write(reinterpret_cast<const char*>(&neuron.Bias), sizeof(neuron.Bias));
        }
    }

    file.close();
}

// Save a neural network to a file using POSIX system calls`
void saveNeuralNetworkToFilePOSIX(const DNN& network, const std::string& filename)
{
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    // Set file access pattern to minimize caching effects
    setNoCachedFile(fd, filename);

    // Output the network's sizes
    uint64_t layersCount = network.Layers.size();
    uint64_t neuronsPerLayer = network.Layers[0].Neurons.size();
    ssize_t bytesWritten1 = write(fd, reinterpret_cast<const char*>(&layersCount), sizeof(layersCount));
    ssize_t bytesWritten2 = write(fd, reinterpret_cast<const char*>(&neuronsPerLayer), sizeof(neuronsPerLayer));
    if (bytesWritten1 <= 0 || bytesWritten2 <= 0)
    {
        std::cerr << "Error writing network dimensions to file: " << filename << std::endl;
        close(fd);
        return;
    }

    uint32_t layerIndex = 0;
    for (const Layer& layer : network.Layers)
    {
        std::string layerTag = "Saving layer " + std::to_string(layerIndex++);
        nvtx3::scoped_range layerRange(layerTag.c_str());

        for (const Neuron& neuron : layer.Neurons)
        {
            for (double weight : neuron.Weights)
            {
                ssize_t bytesWritten3 = write(fd, reinterpret_cast<const char*>(&weight), sizeof(weight));
                if (bytesWritten3 <= 0)
                {
                    std::cerr << "Error writing weight to file: " << filename << std::endl;
                    close(fd);
                    return;
                }
            }

            ssize_t bytesWritten4 = write(fd, reinterpret_cast<const char*>(&neuron.Bias), sizeof(neuron.Bias));
            if (bytesWritten4 <= 0)
            {
                std::cerr << "Error writing bias to file: " << filename << std::endl;
                close(fd);
                return;
            }
        }
    }

    close(fd);
}

void saveNeuralNetworkToFilePOSIXBiggerChunks(const DNN& network, const std::string& filename)
{
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }

    // Set file access pattern to minimize caching effects
    setNoCachedFile(fd, filename);

    // Output the network's sizes
    uint64_t layersCount = network.Layers.size();
    uint64_t neuronsPerLayer = network.Layers[0].Neurons.size();
    ssize_t bytesWritten1 = write(fd, reinterpret_cast<const char*>(&layersCount), sizeof(layersCount));
    ssize_t bytesWritten2 = write(fd, reinterpret_cast<const char*>(&neuronsPerLayer), sizeof(neuronsPerLayer));
    if (bytesWritten1 <= 0 || bytesWritten2 <= 0)
    {
        std::cerr << "Error writing network dimensions to file: " << filename << std::endl;
        close(fd);
        return;
    }

    // Prepare a buffer to hold all neuron data for a single layer
    uint64_t weightsSize = neuronsPerLayer * neuronsPerLayer * sizeof(double);
    uint64_t biasesSize = neuronsPerLayer * sizeof(double);
    uint64_t totalNeuronDataSize = weightsSize + biasesSize;

    double* dataBuffer = reinterpret_cast<double*>(malloc(totalNeuronDataSize));
    if (!dataBuffer)
    {
        std::cerr << "Memory allocation failed\n";
        close(fd);
        return;
    }

    uint32_t layerIndex = 0;
    for (const Layer& layer : network.Layers)
    {
        std::string nvtxCopyMsg = "Copy layer " + std::to_string(layerIndex);
        nvtxRangePushA(nvtxCopyMsg.c_str());

        double* ptr = dataBuffer;

        for (const Neuron& neuron : layer.Neurons)
        {
            for (double weight : neuron.Weights)
            {
                *ptr++ = weight;
            }

            *ptr++ = neuron.Bias;
        }

        nvtxRangePop();

        // Write the entire layer data at once
        std::string nvtxSaveMsg = "Saving layer " + std::to_string(layerIndex);
        nvtxRangePushA(nvtxSaveMsg.c_str());
        
        ssize_t bytesWritten3 = write(fd, reinterpret_cast<const char*>(dataBuffer), totalNeuronDataSize);

        if (bytesWritten3 <= 0)
        {
            std::cerr << "Error writing layer data to file: " << filename << std::endl;
            free(dataBuffer);
            close(fd);
            return;
        }

        nvtxRangePop();
        layerIndex++;
    }

    free(dataBuffer);
    close(fd);
}

void saveNeuralNetworkToFile(const DNN& network, const std::string& filename, IOMode writeMode)
{
    if (writeMode == IOMode::CPP)
        saveNeuralNetworkToFileCpp(network, filename);
    else if (writeMode == IOMode::POSIX)
        saveNeuralNetworkToFilePOSIX(network, filename);
    else if (writeMode == IOMode::POSIX_LARGE_CHUNKS)
        saveNeuralNetworkToFilePOSIXBiggerChunks(network, filename);
    else
        std::cerr << "Unknown write mode specified.\n";
}

// Load a neural network from a file using C++ standard library
DNN loadNeuralNetworkFromFileCpp(const std::string& filename)
{
    DNN network;

    // Open the file containing the neural network
    std::ifstream file(filename, std::ios::binary);
    if (!file)
    {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return network;
    }

    // Read network dimensions
    uint64_t layersCount, neuronsPerLayer;
    file.read(reinterpret_cast<char*>(&layersCount), sizeof(layersCount));
    file.read(reinterpret_cast<char*>(&neuronsPerLayer), sizeof(neuronsPerLayer));

    uint32_t layerIndex = 0;
    for (uint64_t i = 0; i < layersCount; i++) 
    {
        std::string layerTag = "Loading layer " + std::to_string(layerIndex++);
        nvtx3::scoped_range layerRange(layerTag.c_str());

        Layer layer;
        
        for (uint64_t j = 0; j < neuronsPerLayer; j++) 
        {
            Neuron neuron;
            
            // Read weights
            for (uint64_t k = 0; k < neuronsPerLayer; k++) 
            {
                double weight;
                file.read(reinterpret_cast<char*>(&weight), sizeof(weight));
                neuron.Weights.push_back(weight);
            }
            
            // Read bias
            file.read(reinterpret_cast<char*>(&neuron.Bias), sizeof(neuron.Bias));
            
            layer.Neurons.emplace_back(neuron);
        }
        
        network.Layers.emplace_back(layer);
    }

    file.close();
    return network;
}

// Load a neural network from a file using POSIX system calls
DNN loadNeuralNetworkFromFilePOSIX(const std::string& filename)
{
    DNN network;

    // Open the file containing the neural network
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1)
    {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return network;
    }

    setNoCachedFile(fd, filename);

    // Read network dimensions
    uint64_t layersCount, neuronsPerLayer;
    ssize_t bytesRead1 = read(fd, &layersCount, sizeof(layersCount));
    ssize_t bytesRead2 = read(fd, &neuronsPerLayer, sizeof(neuronsPerLayer));
    if (bytesRead1 <= 0 || bytesRead2 <= 0)
    {
        std::cerr << "Error reading network dimensions from file: " << filename << std::endl;
        close(fd);
        return network;
    }
    
    uint32_t layerIndex = 0;
    for (uint64_t i = 0; i < layersCount; i++) 
    {
        std::string layerTag = "Loading layer " + std::to_string(layerIndex++);
        nvtx3::scoped_range layerRange(layerTag.c_str());

        Layer layer;

        for (uint64_t j = 0; j < neuronsPerLayer; j++) 
        {
            Neuron neuron;

            // Read weights
            for (uint64_t k = 0; k < neuronsPerLayer; k++) 
            {
                double weight = 0.0;
                ssize_t bytesRead3 = read(fd, &weight, sizeof(weight));
                if (bytesRead3 <= 0)
                {
                    std::cerr << "Error reading weight from file: " << filename << std::endl;
                    close(fd);
                    return network;
                }

                neuron.Weights.push_back(weight);
            }

            // Read bias
            ssize_t bytesRead4 = read(fd, &neuron.Bias, sizeof(neuron.Bias));
            if (bytesRead4 <= 0)
            {
                std::cerr << "Error reading bias from file: " << filename << std::endl;
                close(fd);
                return network;
            }

            layer.Neurons.emplace_back(neuron);
        }

        network.Layers.emplace_back(layer);
    }

    close(fd);
    return network;
}

// Load a neural network from a file using POSIX system calls with larger chunk reads
DNN loadNeuralNetworkFromFilePOSIXBiggerChunks(const std::string& filename)
{
    DNN network;

    // Open the file containing the neural network
    int fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1)
    {
        std::cerr << "Error opening file for reading: " << filename << std::endl;
        return network;
    }

    setNoCachedFile(fd, filename);

    // Read network dimensions
    uint64_t layersCount, neuronsPerLayer;
    ssize_t bytesRead1 = read(fd, &layersCount, sizeof(layersCount));
    ssize_t bytesRead2 = read(fd, &neuronsPerLayer, sizeof(neuronsPerLayer));
    if (bytesRead1 <= 0 || bytesRead2 <= 0)
    {
        std::cerr << "Error reading network dimensions from file: " << filename << std::endl;
        close(fd);
        return network;
    }

    uint32_t layerIndex = 0;
    for (uint64_t i = 0; i < layersCount; i++) 
    {
        Layer layer;

        std::string nvtxCopyMsg = "Copy layer " + std::to_string(layerIndex);
        nvtxRangePushA(nvtxCopyMsg.c_str());

        uint64_t weightsSize = neuronsPerLayer * neuronsPerLayer * sizeof(double);
        uint64_t biasesSize = neuronsPerLayer * sizeof(double);
        uint64_t totalNeuronDataSize = weightsSize + biasesSize;

        double* dataBuffer = reinterpret_cast<double*>(malloc(totalNeuronDataSize));
        if (!dataBuffer)
        {
            std::cerr << "Memory allocation failed\n";
            close(fd);
            return network;
        }

        nvtxRangePop();

        std::string nvtxSaveMsg = "Loading layer " + std::to_string(layerIndex);
        nvtxRangePushA(nvtxSaveMsg.c_str());

        // Read the entire layer data at once
        ssize_t bytesRead3 = read(fd,reinterpret_cast<void*>(dataBuffer), totalNeuronDataSize);
        if (bytesRead3 <= 0)
        {
            std::cerr << "Error reading layer data from file: " << filename << std::endl;
            free(dataBuffer);
            close(fd);
            return network;
        }

        for (uint64_t j = 0; j < neuronsPerLayer; j++) 
        {
            Neuron neuron;
            
            // Get neuron weights
            for (uint64_t k = 0; k < neuronsPerLayer; k++) 
            {
                neuron.Weights.emplace_back(*dataBuffer++);
            }
            
            // Get the neuron bias
            neuron.Bias = *dataBuffer++;
            
            layer.Neurons.emplace_back(neuron);
        }

        nvtxRangePop();
        
        network.Layers.emplace_back(layer);
    }

    close(fd);
    return network;
}

DNN readNeuralNetworkFromFile(const std::string& filename, IOMode readMode)
{
    if (readMode == IOMode::CPP)
        return loadNeuralNetworkFromFileCpp(filename);
    else if (readMode == IOMode::POSIX)
        return loadNeuralNetworkFromFilePOSIX(filename);
    else if (readMode == IOMode::POSIX_LARGE_CHUNKS)
        return loadNeuralNetworkFromFilePOSIXBiggerChunks(filename);

    return DNN();
}
