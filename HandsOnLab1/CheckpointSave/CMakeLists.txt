# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.10)
project(checkpoint)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_FILES
    src/Main.cpp
    src/SaveAndLoadOperations.cpp
)

include_directories(headers)

add_executable(CheckpointSave ${SOURCE_FILES})
