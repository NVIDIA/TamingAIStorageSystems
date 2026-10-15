# SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: MIT

#pragma once

#include <vector>

struct Neuron 
{
    std::vector<double> Weights;
    double Bias = 0.0;
};

struct Layer
{
    std::vector<Neuron> Neurons;
};

struct DNN 
{
    std::vector<Layer> Layers;
};