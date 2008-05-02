package org.diracvideo.Schroedinger;

public class WaveletTransform {
    public static void inverse(short d[], int w) {
	/* data is assumed to be preinterleaved */
	for(int i = 0; i < d.length; i++) {
	    d[i] += 1;
	    d[i] >>= 1;
	}
    }

    private static void synthesize(short d[]) {
	for(int i = 0; i < d.length; i += 2) {
	    if(i == 0) {
		d[0] -= (2*d[1] + 2) >> 2;
	    } else if (i + 1 == d.length) {
		d[i] -= (2*d[i-1] + 2) >> 2;
	    } else {
		d[i] -= (d[i-1] + d[i+1] + 2) >> 2;
	    }
	}
	for(int i = 0; i < d.length; i += 2) {
	    if(i + 2 >= d.length) {
		d[i+1] += (2*d[i] + 1) >> 1;
	    } else {
		d[i+1] += (d[i] + d[i+2] + 1) >> 1;
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
	short d[] = {0,0,2,0,4,0,6,0,8,1};
	synthesize(d);
	for(int i = 0; i < d.length; i++) {
	    System.out.print(" " + d[i]);
	}
	System.out.println("");
    }
    
}
