#!/bin/bash

LD_LIBRARY_PATH=/usr/local/lib
gst-launch videotestsrc ! schroenc intra_wavelet=5 \
	inter_wavelet=1 enable_noarith=false transform_depth=4  \
	gop_structure=1 rate_control=3 mv_precision=0 ! \
	filesink location=test.drc &
sleep 5
kill -s 2 %gst-launch
exit
