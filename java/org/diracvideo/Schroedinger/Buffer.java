package org.diracvideo.Schroedinger;

public final class Buffer {
    public final int b,e;
    public final byte[] d;

    public Buffer(byte d[], int b, int e) {
	this.b = clamp(b,0,d.length - 1);
	this.e = clamp(e,b,d.length - 1);
	this.d = d;
    }

    public Buffer(byte d[], int b) {
	this(d,b, d.length - 1);
    }

    private int clamp(int i, int l, int h) {
	return (i < l ? l : i > h ? h : i);
    }
}