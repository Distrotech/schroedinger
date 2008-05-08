package org.diracvideo.Schroedinger;

/* pictures in the dirac stream specification can basically parse themselves
   we are going to take advantage of that */

public class Picture {
    Unpack u;
    WaveletTransform w;
    Parameters p;
    int n;
    

    public Picture(byte d[]) {
	u = new Unpack(d);
	w = new WaveletTransform();
    }

    public void parse() {

    }
    
    public void decode() {

    }
    
}
