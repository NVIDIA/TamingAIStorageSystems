#!/usr/bin/env bash

# SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: MIT

# 3-advanced-striping/pfl-example.sh
# Progressive File Layout (PFL): the stripe pattern CHANGES as the file grows.
# One policy that fits a 4 KB config AND a 4 TB checkpoint. Read, don't run.
set -euo pipefail

# Define composite regions with -E (end of each extent):
#   0    .. 1 MiB : 1 stripe   (small files & file heads stay cheap)
#   1MiB .. 1 GiB : 4 stripes  (mid files get some width)
#   1GiB .. EOF   : all OSTs   (huge files fan out fully)
lfs setstripe \
    -E 1M   -c 1  \
    -E 1G   -c 4  \
    -E -1   -c -1 \
    /scratch/mixed/

# Inspect a composite file -- getstripe now prints one block PER component:
lfs getstripe /scratch/mixed/checkpoint.bin
#   lcm_layout_gen:    3
#   lcm_mirror_count:  1
#   lcm_entry_count:   3
#     lcme_id:             1
#     lcme_flags:          init
#     lcme_extent.e_start: 0
#     lcme_extent.e_end:   1048576
#       lmm_stripe_count:  1
#       lmm_stripe_size:   1048576
#     lcme_id:             2
#     lcme_extent.e_start: 1048576
#     lcme_extent.e_end:   1073741824
#       lmm_stripe_count:  4
#     lcme_id:             3
#     lcme_extent.e_start: 1073741824
#     lcme_extent.e_end:   EOF
#       lmm_stripe_count:  -1

# WHY: users stop having to guess. The layout follows file size automatically.
