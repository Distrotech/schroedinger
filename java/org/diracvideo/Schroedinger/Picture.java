package org.diracvideo.Schroedinger;

/* pictures in the dirac stream specification can basically parse themselves
   we are going to take advantage of that */

public class Picture {
    Unpack u;
    WaveletTransform w;
    VideoFormat f;
    Parameters p;
    int n;

    private class Parameters {
	public final int iwt_chroma_width, iwt_chroma_height,
	    iwt_luma_width, iwt_luma_height;
	public final int xblen_luma, yblen_luma, 
	    xbsep_luma, ybsep_luma;
	public final boolean have_global_motion;
	public Parameters() {
	    have_global_motion = false; 
	    iwt_chroma_width = 0;
	    iwt_chroma_height = 0;
	    iwt_luma_height = 0;
	    iwt_luma_width = 0;
	    xblen_luma = yblen_luma = 0;
	    xbsep_luma = ybsep_luma = 0;
	}
    }    

    public Picture(byte d[], VideoFormat f) {
	u = new Unpack(d);
	w = new WaveletTransform();
	p = new Parameters();
	this.f = f;
    }

    public void parse() {

    }
    
    public void decode() {

    }
    
}
