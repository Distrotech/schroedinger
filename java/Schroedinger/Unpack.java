package Schroedinger;

public class Unpack {
    private char d[];
    private int i = 0,r, l = 0;
    
    public Unpack(char d[]) {
	this.d = d;
    }
    
    private void fill() {
	r = (d[i] << 24) | (d[i+1] << 16) | (d[i+2] << 8) | (d[i+3]);
	i += 4;
	l = 32;
    }
    
    public void align() {
	l -= (l & 7);
	fill();
    }

    public String toString() {
	return "Unpack Object";
    }
    
}