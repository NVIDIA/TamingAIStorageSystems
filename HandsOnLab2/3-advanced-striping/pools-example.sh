#!/usr/bin/env bash

# SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: MIT

# 3-advanced-striping/pools-example.sh
# OST pools: name a subset of OSTs, then point policy at it. Combine with PFL
# so "which media" and "how wide" are both automatic per directory.
set -euo pipefail

# Define pools once (admin action):
lctl pool_new     ai0.flash
lctl pool_add     ai0.flash ai0-OST[0000-0007]     # 8 NVMe OSTs
lctl pool_new     ai0.capacity
lctl pool_add     ai0.capacity ai0-OST[0008-0027]  # 32 HDD/QLC OSTs

# Hot scratch: wide stripe on the flash pool.
lfs setstripe -p ai0.flash    -c 8  /scratch/hot/

# Cold datasets: PFL on the capacity pool (narrow head, wide tail).
lfs setstripe -E 1G -c 1 -p ai0.capacity -E -1 -c -1 -p ai0.capacity /datasets/cold/
# (in a PFL layout the pool, like -c, is set per component, after its -E)

# Best practice: pools + PFL means users never choose media or width by hand.
