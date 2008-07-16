package org.diracvideo.Schroedinger;

/** Wavelet:
 *
 * The class for doing wavelet transformations.
 * Should be refactored to provide actual objects
 * (functors) with two methods: transform(short[],int, int)
 * and inverse(short[],int, int)
 * We will not use 2-dimensional arrays as it is slow
 * and we need no such handholding. */

public class Wavelet {
    /** inverse:
     * @param data   a short[] array containing the frame
     * @param width  width of the frame
     * @param depth  transform depth
     *
     * Inverse is the only method users should ever call, except possibly 
     * for interleave, which interleaves four subbands so that it is ready
     * to be used for inverse(). The actual picture decoding sequence never
     * calles interleave() though.  **/
    public static void inverse(short data[], int w, int depth) {
	/* data is assumed to be preinterleaved */
	for(int s = (1 << (depth - 1)); s > 0; s >>= 1) {
	    for(int x = 0; x < w; x += s) {
		synthesize(data,s*w,x,data.length); /* a column */
	    }
	    for(int y = 0; y < data.length; y += w*s) {
		synthesize(data,s,y, y + w); /* a row */
	    }
            for(int y = 0; y < data.length; y += s*w) {
                for(int x = 0; x < w; x += s) {
	            data[y+x] = (short)((data[y+x]+1)>>1);
		}
	    }
	}
    }
    /** synthesize:
     *
     * @param d   short[] data array
     * @param s   stride
     * @param b   begin
     * @param e   end
     *
     *  This method is public for testing purposes only. */
    public static void synthesize(short d[], int s, int b, int e) {
	for(int i = b; i < e; i += 2*s) {
	    if(i - s < b) {
		d[i] -= (d[i+s] + 1) >> 1;
	    } else {
		d[i] -= (d[i-s] + d[i+s] + 2) >> 2;
	    }
	}
	for(int i = b + s; i < e; i += 2*s) {
	    if(i + s >= e) {
		d[i] += d[i-s];
	    } else {
		d[i] += (d[i-s] + d[i+s] + 1) >> 1;
	    }
	}
    }

    private static void
	synthesizeDeslausriesDebuc(short d[], int s, int b, int e) {
	for(int i = b; i < e; i += 2*s) {
	    if(i - s < b) {
		d[i] -= (d[i+s] + 1) >> 1;
	    } else {
		d[i] -= (d[i-s] + d[i+s] + 2) >> 2;
	    }
	}
	for(int i = b + s; i < e; i+= 2*s) {
	    d[i] += (9*d[i-s] + 9*d[i+s] - d[i-3*s] - d[i+3*s] + 8) >> 4;
	}
    }
    /**
     * interleave interleaves four subbands.
     * @param ll array containing the subband data (like hl, lh, hh)
     * @param width: the width of each subband **/
    public static short[] interleave(short ll[], short hl[], 
				     short lh[], short hh[], int width) {
	short o[] = new short[ll.length*4];
	int height = ll.length / width;
	for(int y = 0; y < height; y++) {
	    for(int x = 0; x < width; x++) {
		int pos = (x + y*width);
		int outpos = 2*x + 4*y*width;
		o[outpos] = ll[pos];
		o[outpos+1] = hl[pos];
		o[outpos+2*width] = lh[pos];
		o[outpos+2*width+1] = hh[pos];
	    }
	}
	return o;
    }

}
