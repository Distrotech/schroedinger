package org.diracvideo.Schroedinger;

final class Buffer {
    public final int b,e;
    public final byte[] d;

    public Buffer(byte d[], int b, int e) {
	this.b = Util.clamp(b,0,d.length - 1);
	this.e = Util.clamp(e,b,d.length - 1);
	this.d = d;
    }

    public Buffer(byte d[], int b) {
	this(d,b, d.length - 1);
    }

    public Buffer sub(int b, int e) {
	return new Buffer(this.d, this.b + b, this.b + e);
    }

}