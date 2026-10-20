#!/usr/bin/env bash

# SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: MIT

# 2-striping/commands.sh
# The exact sequence Lab 1 walks. READ it; the outputs are pre-recorded in
# before.out / after.out. Do not run against a real filesystem in the room.
set -euo pipefail

CKPT=/scratch/ckpt/model_epoch3.pt   # a multi-GB training checkpoint

# --- STEP 1: inspect the default. This is the trap. ---------------------------
lfs getstripe "$CKPT"
#   lmm_stripe_count: 1        <- whole file on ONE OST
#   lmm_stripe_size:  1048576  <- 1 MiB

# --- STEP 2: widen the stripe on the directory, then re-write the file --------
# setstripe sets policy on a DIRECTORY; new files inherit it. Existing files
# keep their layout until rewritten (copy out + back, or use lfs migrate).
lfs setstripe -c 8  /scratch/ckpt/     # spread new files across 8 OSTs
# ... application re-writes the checkpoint ...  -> results captured in after.out

# --- STEP 3: go all the way -- stripe across every OST -------------------------
lfs setstripe -c -1 /scratch/ckpt/     # -1 = all 40 OSTs
# ... re-write again ...                -> results captured in after.out

# --- STEP 4: confirm the layout actually changed ------------------------------
lfs getstripe /scratch/ckpt/model_epoch3.pt
#   lmm_stripe_count: 8  (or 40)   lmm_stripe_size: 1048576

# --- Migrate in place instead of copying (handy on real systems) --------------
#   lfs migrate -c 8 "$CKPT"
