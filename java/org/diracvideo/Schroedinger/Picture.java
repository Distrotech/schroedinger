package org.diracvideo.Schroedinger;

/* pictures in the dirac stream specification can basically parse themselves
   we are going to take advantage of that */

public class Picture {
    private Buffer b;
    private WaveletTransform w;
    private VideoFormat f;
    private Parameters p;
    private int c;
    public final int n;

    private class Parameters {
	public final int iwt_chroma_width, iwt_chroma_height,
	    iwt_luma_width, iwt_luma_height;
	public final int xblen_luma, yblen_luma, 
	    xbsep_luma, ybsep_luma;
	public final int num_refs;
	public final boolean have_global_motion;
	public Parameters(int c) {
	    num_refs = c & 0x02;
	    have_global_motion = false; 
	    iwt_chroma_width = 0;
	    iwt_chroma_height = 0;
	    iwt_luma_height = 0;
	    iwt_luma_width = 0;
	    xblen_luma = yblen_luma = 0;
	    xbsep_luma = ybsep_luma = 0;
	}
    }    

    public Picture(int c, int n, Buffer b, VideoFormat f) {
	this.b = b;
	this.f = f;
	this.n = n;
	this.c = c;
    }

    public void parse() {
	this.p = new Parameters(c);
	this.w = new WaveletTransform();

    }
    
    public void decode() {

    }
    
}
