#!/bin/bash

gst-launch videotestsrc ! schroenc intra_wavelet=1 \
	    enable_noarith=true transform_depth=4  \
	    gop_structure=1 rate_control=3 ! \
	     filesink location=test.drc &
sleep 5
kill -s 2 %gst-launch
exit
