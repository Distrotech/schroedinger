package org.diracvideo.Schroedinger;

public class ColourSpace {
    private double matrix[][];
    private VideoFormat format;
    public ColourSpace(int colourmode, VideoFormat f) {
	matrix = new double[3][3];
	format = f;
	setupMatrix(0.2990, 0.1140);
    }
    
    private void setupMatrix(double kr, double kb) {
	double kg = 1.0 - kr - kb;
	matrix[0][0] = matrix[1][0] = matrix[2][0] = 1;
	matrix[0][2] = 2*(1-kr);
	matrix[1][1] = -2*kb*(1-kb)/kg;
	matrix[1][2] = -2*kr*(1-kr)/kg;
	matrix[2][1] = 2*(1-kb);
    }
    
    private void applyMatrix(int in[], int out[]) {
	for(int i = 0; i < 3; i++) {
	    double x = 0;
	    x += matrix[i][0] * in[0];
	    x += matrix[i][1] * in[1];
	    x += matrix[i][2] * in[2];
	    out[i] = Util.clamp(x,0,255);
	}
    }
    
    private void clampYuv(int yuv[]) {
       	yuv[0] = Util.clamp(yuv[0] + 128, format.luma_offset, format.luma_offset + format.luma_excursion);
    }

    public final int convert(short y, short u, short v) {
	int rgb[] = new int[3], yuv[] = {y,u,v};
	/*	clampYuv(yuv);
		applyMatrix(yuv,rgb);*/
	return Util.clamp(y+32,0,255)*0x010101;
    }
}
















