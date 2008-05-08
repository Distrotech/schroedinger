package org.diracvideo.Schroedinger;


class Parameters {
    public final int iwt_chroma_width, iwt_chroma_height,
	iwt_luma_width, iwt_luma_height;
    public final int xblen_luma, yblen_luma, 
	xbsep_luma, ybsep_luma;
    public final boolean have_global_motion;
    public Parameters(byte d[]) {
	Unpack u = new Unpack(d);
	have_global_motion = false; 
	iwt_chroma_width = 0;
	iwt_chroma_height = 0;
	iwt_luma_height = 0;
	iwt_luma_width = 0;
	xblen_luma = yblen_luma = 0;
	xbsep_luma = ybsep_luma = 0;
    }
}