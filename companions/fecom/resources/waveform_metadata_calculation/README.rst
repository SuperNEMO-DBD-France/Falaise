=============================
Waveform metadata calculation
=============================
:Authors: G.Oliviéro <goliviero@lpccaen.in2p3.fr>,
:Date:    2017

These python programs take waveforms from commissionings to recalcul all metadata from the raw waveform given by the front-end board.

It ensures that we can recalcul offline all the metadata calculation done in the calo front-end boards.

We also can use these programs to optimize the metadata (for example the baseline, we can take more than 16 samples to do our future calculation. These modifications can't be done in the front-end board, it is 16 samples but we can recalcul and chane this number of samples offline).
