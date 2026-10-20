#!/usr/bin/env bash

# SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: MIT

# 4-metadata/dne-example.sh
# DNE = Distributed Namespace. Spread METADATA load across multiple MDTs so a
# job opening millions of small files isn't bottlenecked on one metadata server.
set -euo pipefail

# DNE phase 1: remote directories -- a whole directory lives on a chosen MDT.
lfs mkdir -i 1 /ai0/team-b        # place team-b's tree on MDT index 1

# DNE phase 2: STRIPED directories -- entries of ONE directory are hashed across
# several MDTs. This is what a 2,000,000-file training input needs.
lfs mkdir -c 2 -i 0 /ai0/dataset/imagenet   # stripe this dir across 2 MDTs

lfs getdirstripe /ai0/dataset/imagenet
#   lmv_stripe_count: 2   lmv_stripe_offset: 0
#   mdtidx 0  ...
#   mdtidx 1  ...

# statahead: readahead for metadata. Raise depth so mass opens pipeline instead
# of stalling one lookup at a time.
lctl set_param llite.ai0*.statahead_max=64

# WATCH the balance change (compare to 1-base-config/lfs-df.out where MDT:1
# was nearly empty):  lfs df -i    # -i shows inodes per MDT
