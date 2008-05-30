package org.diracvideo.Schroedinger;

/* pictures in the dirac stream specification can basically parse themselves
   we are going to take advantage of that */

public class Picture {
    private Buffer b;
    private WaveletTransform w;
    private VideoFormat f;
    private Parameters p;
    private int c;
    private Exception e;
    private Decoder.Status s = Decoder.Status.OK;
    public final int n;



    private static class Parameters {
	/* all that matters for now is wavelet depth */
	public int iwt_chroma_width, iwt_chroma_height,
	    iwt_luma_width, iwt_luma_height;
	public int transform_depth;
	public WaveletTransform wavelet;
	public boolean is_noarith;
	public int num_refs;
	public Parameters(int c, VideoFormat f) {
	    is_noarith = (c & 0x48) == 0x8;
	    num_refs = (c & 0x3);
	    iwt_chroma_width = 0;
	    iwt_chroma_height = 0;
	    iwt_luma_height = 0;
	    iwt_luma_width = 0;
	}
    }    

    public Picture(int c, int n, Buffer b, VideoFormat f) {
	this.b = b;
	this.f = f;
	this.n = n;
	this.c = c;
	this.p = new Parameters(c,f);
    }

    public void parse() {
	Unpack u = new Unpack(b);
	
    }
    
    public void decode() {

    }
    
}
