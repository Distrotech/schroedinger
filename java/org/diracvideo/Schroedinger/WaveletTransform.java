package org.diracvideo.Schroedinger;

public class WaveletTransform {
    public static void inverse(short d[], int w) {
	/* data is assumed to be preinterleaved */
	for(int s = 8; s >= 1; s >>= 1) {
	    for(int x = 0; x < w; x++) {
		synthesize(d,s*w,x,d.length); /* a column */
	    }
	    for(int y = 0; y < d.length; y += w) {
		synthesize(d,s,y,w); /* a row */
	    }
	}
	for(int i = 0; i < d.length; i++) {
	    d[i] += 1;
	    d[i] >>= 1;
	}
    }

    private static void synthesize(short d[], int s, int b, int e) {
	for(int i = b; i < e; i += 2*s) {
	    if(i == 0) {
		d[0] -= (2*d[s] + 2) >> 2;
	    } else if (i + s >= e) {
		d[i] -= (2*d[i-s] + 2) >> 2;
	    } else {
		d[i] -= (d[i-s] + d[i+s] + 2) >> 2;
	    }
	}
	for(int i = b + s; i < e; i += 2*s) {
	    if(i + s >= e) {
		d[i] += (2*d[i-s] + 1) >> 1;
	    } else {
		d[i] += (d[i-s] + d[i+s] + 1) >> 1;
	    }
	}
    }

    public static short[] interleave(short ll[], short hl[], 
				     short lh[], short hh[], int w) {
	short o[] = new short[ll.length*4];
	for(int i = 0; i < ll.length; i++) {
	    for(int j = 0; j < ll.length; j++) {
		int pos =(w*j + i);
		o[2*pos] = ll[pos];
		o[2*pos + 1] = hl[pos];
		o[2*pos + w] = lh[pos];
		o[2*pos + w + 1] = hh[pos];
	    }
	}
	return o;
    }

    public static void test() {
	short d[] = {0,0,0,0,0,0,0,0,8,1};
	synthesize(d,4,0,d.length);
	synthesize(d,2,0,d.length);
	synthesize(d,1,0,d.length);
	for(int i = 0; i < d.length; i++) {
	    System.out.print(" " + d[i]);
	}
	System.out.println("");
    }
    
}
