package org.diracvideo.Schroedinger;

final class Util {
    public final static int roundUpPow2(int x, int y) {
	return (((x) + (1<<(y)) - 1)&((~0)<<(y)));
    }

    public final static int roundUpShift(int x, int y) {
	return (((x) + (1<<(y)) - 1)>>(y));
    }

    public final static int divideRoundUp(int x, int y) {
	return (x + y - 1)/y;
    }

    public final static int clamp(int i, int l, int h) {
	return (i < l ? l : i > h ? h : i);
    }

    public final static int clamp(double i, int l, int h) {
	int v = (int)(i+0.5);
	return clamp(v,l,h);
    }

    public final static int median(int a[]) {
	if(a[0] < a[1]) {
	    if(a[1] < a[2]) return a[1];
	    if(a[2] < a[0]) return a[0];
	    return a[2];
	} else {
	    if(a[0] < a[2]) return a[0];
	    if(a[2] < a[1]) return a[1];
	    return a[2];
	}
    }

    public final static int mean(int ... numbers) {
	int s = 0;
	for(int i = 0; i < numbers.length; i++)
	    s += numbers[i];
	return s/numbers.length;
    }

    public final static short getRamp(int x, int offset) {
	if(offset == 1) {
	    if(x == 0) return 3;
	    return 5;
	}
	return (short)(1 + (6 * x + offset - 1)/(2*offset - 1));
    }
}

