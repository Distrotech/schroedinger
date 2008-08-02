package org.diracvideo.Schroedinger;

public class WaveletTransform {
    public static void inverse(short d[], int w) {
	/* data is assumed to be preinterleaved */
	int h = d.length / w;
	for(int s = 8; s  > 0; s >>= 1) {
	    for(int x = 0; x < w; x++) {
		synthesize(d, x, w*x*s, -1); /* all columns */
	    }
	    for(int y = 0; y < h; y++) {
		synthesize(d,y*w,s,-1); /* all rows */
	    }
	}
	/* shift bits */
	for(int i = 0; i < d.length; i++) {
	    d[i] += 1;
	    d[i] >>= 1;
	}
    }

    private static void synthesize(short data[], int start,
				   int stride, int width) {
	for(int i = 0; i < width; i += stride) {
	    if(i == 0) {
		data[0] -= (data[1] + 2) >> 2;
	    } else if (i == width - 1) {
		data[2*i] -= (data[2*i-1] + 2) >> 2;
	    } else {
		data[2*i] -= (data[2*i-1] + data[2*i+1] + 2) >> 2;
	    }
	}
	for(int i = 0; i < width; i += stride) {
	    if(i+1 > width) {
		data[2*i+1] += (data[2*i] + 1) >> 1;
	    } else { 
		data[2*i+1] += (data[2*i] + data[2*i+2] + 1) >> 1;
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

    
}
