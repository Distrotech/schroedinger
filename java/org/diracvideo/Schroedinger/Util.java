package org.diracvideo.Schroedinger;

final class Util {
    public final static int roundUpPow2(int x, int y) {
	return (((x) + (1<<(y)) - 1)&((~0)<<(y)));
    }

    public final static int roundUpShift(int x, int y) {
	return (((x) + (1<<(y)) - 1)>>(y));
    }
}

