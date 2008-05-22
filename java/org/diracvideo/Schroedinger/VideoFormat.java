package org.diracvideo.Schroedinger;

public class VideoFormat {
    public int width, height;
    public enum ChromaFormat {
	CHROMA_444, CHROMA_422, CHROMA_420
    }
    public ChromaFormat chroma_format;

    public boolean interlaced, top_field_first;
    public int frame_rate_numerator, frame_rate_denominator,
	aspect_ratio_numerator, aspect_ration_denominator;
    public int clean_width, clean_height, left_offset, top_offset;
    public int luma_offset, luma_excursion, chroma_offset, chroma_excursion;
    

    public VideoFormat(Buffer b) {
	
    }

    public VideoFormat defaultFormat(int n) {
	return new VideoFormat(null);
    }
}
