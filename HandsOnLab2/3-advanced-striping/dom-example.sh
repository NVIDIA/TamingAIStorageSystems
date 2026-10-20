#!/usr/bin/env bash

# SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: MIT

# 3-advanced-striping/dom-example.sh
# Data-on-MDT (DoM): keep the tiny-file TAIL on the metadata plane's flash,
# so small files never pay an OST round-trip. Great for config/label/JSON storms.
set -euo pipefail

# First component lives on the MDT (-L mdt); the rest spills to OSTs via PFL.
# DoM component size must be a multiple of 64 KiB (the min stripe size).
lfs setstripe \
    -E 64K  -L mdt \
    -E -1   -c 4   \
    /scratch/smallfiles/

lfs getstripe -v /scratch/smallfiles/labels.json
#   component0: pattern mdt   (data lives ON the MDT, no OST access)
#   component1: pattern raid0  stripe_count 4  (only if file grows past 64 KiB)

# TRADE: DoM consumes MDT flash and MDT is a scarcer resource than OST capacity.
# Use it where small-file open/read latency dominates -- not everywhere.
