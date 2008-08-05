package org.diracvideo.Schroedinger;
import java.awt.Point;
import java.awt.Dimension;


/** Block
 *
 * Has methods for getting the correct positions
 * of elements in the data array. I would call it
 * Frame but that conflicts with java.awt.Frame. */

public final class Block {
    public final short[] d;
    public final Point p;
    public final Dimension s, o;
    /** Default Block constructor 
     * 
     * @param d is the data of the frame
     * @param p is the place where the frame should start
     * @param s is the dimension of the frame
     * @param o is the dimension of the outer frame **/
    
    public Block(short d[], Point p, Dimension s, Dimension o) {
	this.s = s;
	this.d = d;
	this.p = p;
	this.o = o;
    }
    /**
     * Creates a Block consisting of the entire frame.
     *
     * @param width is the width of the frame
     * @param d is the data of the frame **/

    public Block(short d[], int width) {
	this.d = d;
	this.p = new Point(0,0);
	this.s = this.o = new Dimension(width, d.length / width);
    }
    
    /**
     * Creates a Block with dimension d using a 
     * newly allocated array for the frame **/
    public Block(Dimension d) {
	this.d = new short[d.width * d.height];
	this.o = this.s = d;
	this.p = new Point(0,0);
    }

    public Block sub(Point off, Dimension sub) {
	Point pnt = new Point(p.x + off.x, p.y + off.y);
	return new Block(d, pnt, sub, o);
    }

    /**
     * @return the index to the start of the frame **/
    public int start() {
	return (p.y*o.width) + p.x;
    }

    /**
     * @return the index to the end of the framE (last element + 1) **/
    public int end() {
	return line(s.height - 1) + s.width;
    }
    

    /**
     * @return the line of the frame with index n **/
    public int line(int n) {
	n = Util.clamp(n,0, s.height - 1) + p.y;
	return (n*o.width) + p.x;
    }

    /** The index of a point
     * @return the index for a general point in the frame **/
    public int index(int x, int y) {
	return line(y) + Util.clamp(x,0, s.width - 1);
    }

    /** Pixel at a given point, unchecked */
    public short pixel(int x, int y) {
	return d[(y + p.y)*o.width + (p.x + x)];
    }

    public void copyTo(Block b) {
	try {
	    for(int i = 0; i < s.height; i++) {
		System.arraycopy(d, line(i), b.d, b.line(i), b.s.width);
	    }
	} catch(IndexOutOfBoundsException x) {}
    }
    

    /** A test method which fills the block with a checkers pattern */
    public void checkers(int m) {
	m = (1 << m);
	for(int i = 0; i < s.height; i++) {
	    for(int j = 0; j < s.width; j++) {
		d[index(i,j)] = (short)(((i&m)^(j&m))*255);
	    }
	}
    }
    
    public boolean equals(Block o) {
	if(s.width != o.s.width)
	    return false;
	if(s.height != o.s.height) 
	    return false;
	for(int i = 0; i < s.height; i++)
	    for(int j = 0; j < s.width; j++) 
		if(pixel(i,j) != o.pixel(i,j))
		    return false;
	return true;
    }
}