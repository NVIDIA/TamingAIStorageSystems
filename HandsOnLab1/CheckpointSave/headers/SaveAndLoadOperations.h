# SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include "DNN.h"

enum class IOMode 
{
    POSIX,
    POSIX_LARGE_CHUNKS,
    CPP,
    UNKNOWN
};

std::string IOModeToString(const IOMode& mode);
IOMode stringToIOMode(const std::string& modeStr);

void saveNeuralNetworkToFile(const DNN& network, const std::string& filename, IOMode writeMode);
DNN readNeuralNetworkFromFile(const std::string& filename, IOMode readMode);
