package org.diracvideo.Schroedinger;
import java.awt.Point;
import java.awt.Dimension;
/** Buffer and Block
 *
 * Buffer represents a one-dimensional array,
 * Block represent a two-dimensional array. 
 * They make up for the lack of pointer arithmetic 
 * in java.
 */
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

/** Block
 *
 * Has methods for getting the correct positions
 * of elements in the data array */


final class Block {
    public final short[] d;
    public final Point p;
    public final Dimension s, o;
    public Block(short d[], Point p, Dimension s, Dimension o) {
	this.s = s;
	this.d = d;
	this.p = p;
	this.o = o;
    }

    public int start() {
	return (p.y*o.width) + p.x;
    }

    public int end() {
	return line(s.height - 1) + s.width;
    }
    
    public int line(int n) {
	n = Util.clamp(n,0, s.height - 1) + p.y;
	return (n*o.width) + p.x;
    }

    public int index(int x, int y) {
	return line(y) + Util.clamp(x,0, s.width - 1);
    }
    
}