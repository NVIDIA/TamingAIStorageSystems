==============================================================================
 SC26 Tutorial — Taming AI Storage Systems
 Hands-on Lab 2: Storage System Configuration   (9:40–9:50, 10 min)
 Folder: HandsOnLab2/   (sits beside HandsOnLab1/ and HandsOnLab3/ in the tutorial download tree)
 Presenter: Christopher Grossmeier (DDN)
==============================================================================

WHAT THIS IS
  A follow-along package for Hands-on Lab 2. You need nothing but a TEXT EDITOR.
  There is no live cluster and no software to install. Every performance
  effect is shown through pre-recorded output files you open and read.

  Worked example: EXAScaler / Lustre. The mechanics are Lustre / parallel-FS
  mechanics and transfer conceptually to any parallel file system. This is
  mechanics, not a product pitch.

WHAT WE COVER LIVE vs. TAKE-HOME
  The live slot is 10 minutes, so in the room we walk ONLY:
      2-striping/           the default trap and the before/after fix
      3-advanced-striping/  PFL + the striping trade-off (briefly)
  Everything else (1-base-config/, 4-metadata/) is take-home reference you
  can work through at your own pace. Same files, same order.

HOW TO WORK THROUGH IT (open folders in this order)
  0-reference/         Handout tables you can keep. Skim first, refer back.
      settings-tables.pdf   every knob: default, range, impact
      systems-matrix.pdf    who / what / when / why / how, per system
      decision-table.pdf    workload -> best fit
  1-base-config/       Read the system before you touch it.
      exascaler-annotated.conf   commented config, every line explained
      lctl-dl.out                captured device list, read along
      lfs-df.out                 captured capacity per target
      cluster-map.png            the anatomy diagram (three planes)
  2-striping/          The big lever.
      commands.sh                exact lfs setstripe / getstripe sequence
      before.out                 stripe_count = 1  (the default trap)
      after.out                  stripe_count = 8 and -1 (all OSTs)
      results.csv                the same numbers, for charting
  3-advanced-striping/ Files are not all one size.
      pfl-example.sh             Progressive File Layout
      dom-example.sh             Data-on-MDT for the tiny-file tail
      pools-example.sh           OST pools + PFL so policy is automatic
      results-summary.txt        what each layout buys you
  4-metadata/          The interesting part: metadata, security, governance.
      dne-example.sh             Distributed Namespace (multiple MDTs)
      nodemap-example.txt        multi-tenant UID mapping
      project-quota.txt          per-team accounting
      changelog-sample.out       audit trail: who touched what

A NOTE ON THE NUMBERS
  Bandwidth and latency figures in the .out / .csv files are RELATIVE and
  synthetic, expressed as multiples of a stripe_count=1 baseline. They exist
  to show the SHAPE of the effect, not to benchmark any specific product.

THE ONE HABIT TO TAKE HOME
  Ask "what workload?" before "what setting?" -- then measure before and
  after every change. A relative delta is enough to prove the point.
