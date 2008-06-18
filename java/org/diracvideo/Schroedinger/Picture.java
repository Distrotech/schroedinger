package org.diracvideo.Schroedinger;

/* pictures in the dirac stream specification can basically parse themselves
   we are going to take advantage of that */

public class Picture {
    private Buffer b;
    private WaveletTransform w;
    private Decoder d;
    private VideoFormat f;
    private Parameters p;
    private int c;
    private Exception e;
    private Decoder.Status s = Decoder.Status.OK;
    public final int n;



    private static class Parameters {
	/* all that matters for now is wavelet depth */
	public int iwt_chroma_size[], iwt_luma_size[];
	public int transform_depth = 4;
	public boolean is_noarith;
	public int num_refs;
	public Parameters(int c) {
	    is_noarith = (c & 0x48) == 0x8;
	    num_refs = (c & 0x3);
	}

	public void calculateIwtSize(VideoFormat f) {
	    int size[] = new int[2];
	    iwt_luma_size = new int[2];
	    iwt_chroma_size = new int[2];
	    f.getPictureLumaSize(size);
	    iwt_luma_size[0] = Util.roundUpPow2(size[0], transform_depth);
	    iwt_luma_size[1] = Util.roundUpPow2(size[1], transform_depth);
	    f.getPictureChromaSize(size);
	    iwt_chroma_size[0] = Util.roundUpPow2(size[0], transform_depth);
	    iwt_chroma_size[1] = Util.roundUpPow2(size[1], transform_depth);
	}
    }    

    public Picture(int code, int num, Buffer buf, Decoder dec) {
	n = num;
	c = code;
	b = buf;
	d = dec;
	f = dec.getVideoFormat();
	p = new Parameters(c);
	System.err.format("Picture with code %02X and number %d\n", code, num);
    }

    public void parse() {
	Unpack u = new Unpack(b);
	
    }
    
    public void decode() {

    }
    
}
