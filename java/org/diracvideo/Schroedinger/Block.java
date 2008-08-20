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

    /** @return the index to the start of the frame **/
    public int start() {
	return (p.y*o.width) + p.x;
    }

    /** @return the index to the end of the frame (last element + 1) **/
    public int end() {
	return line(s.height - 1) + s.width;
    }
    
    /** @return the line of the frame with index n **/
    public int line(int n) {
	return (n+p.y)*o.width + p.x;
    }

    /** The index of a point
     * @return the index for a general point in the frame **/
    public int index(int x, int y) {
	return line(y) + x;
    }

    /** Pixel at a given point, unchecked */
    public short pixel(int x, int y) {
	return d[(y + p.y)*o.width + (p.x + x)];
    }

    /** Pixel at a given point, checked */
    public short real(int x, int y) {
	return pixel(Util.clamp(x, 0, s.width - 1),
		     Util.clamp(y, 0, s.height - 1));
    }

    public void set(int x, int y, short v) {
	d[(y+p.y)*o.width + (p.x + x)] = v;
    }
    
    public void set(int x, int y, int v) {
	set(x, y, (short)v);
    }

    public void addTo(Block b) {
	int height = Math.min(b.s.height, s.height);
	int width = Math.min(b.s.width, s.width);
	for(int y = 0; y < height; y++) {
	    for(int x = 0; x < width; x++) 
		b.set(x, y, (short) (pixel(x, y) + b.pixel(x, y)));
	}
    }
    
    /** upsample a block
     * block should be `real'
     * @return the upsampled block
     * @see the dirac specification section 15.8.11 */
    public Block upSample() {
	Block r = new Block(new Dimension(2*s.width, 2*s.height));
	short taps[] = {21, -7, 3, -1};
	for(int y = 0; y < s.height - 1; y++) { /* vertical upsampling */
	    for(int x = 0; x < s.width - 1; x++) {
		r.set(x*2, y*2, pixel(x,y)); /* the copying part */
	    }
	    for(int x = 0; x < s.width - 1; x++) {
		short val = 16;
		for(int i = 0; i < 4; i++) {
		    val += taps[i]*pixel(x, Math.max(0, y - i));
		    val += taps[i]*pixel(x, Math.min(s.height - 1, y + i));
		}
		r.set(x*2, y*2 + 1, (short)(val >> 5));
	    }
	}
	for(int y = 0; y < s.height - 1; y++) {
	    for(int x = 0; x < s.width - 1; x++) {
		short val = 16;
		for(int i = 0; i < 4; i++) {
		    val += taps[i]*pixel(Math.max(0, x - i), y);
		    val += taps[i]*pixel(Math.min(x + i, s.width - 1), y);
		}
		r.set(x*2 + 1, y*2, (short) (val >> 5));
		val = 16;
		for(int i = 0; i < 4; i++) {
		    int xdown = Math.max(0, (x-i)*2);
		    int xup = Math.min(2*s.width - 2, (x+i)*2);
		    val += taps[i]*r.pixel(xdown, y*2+1);
		    val += taps[i]*r.pixel(xup, y*2+1);
		}
		r.set(x*2 + 1, y*2 + 1, (short) (val >> 5));
	    }
	}
	return r;
    }

    public void shiftOut(int b, int a) {
	for(int y = 0; y < s.height; y++)
	    for(int x = 0; x < s.width; x++)
		set(x, y, (short)((pixel(x, y) + a) >> b));
    }

    public void clip(int b) {
	int l = -(1 << b), h = (1 << b) - 1;
	for(int y = 0; y < s.height; y++) {
	    int line = line(y);
	    for(int x = 0; x < s.width; x++) 
		d[line+x] = (short)Util.clamp(d[line+x], l, h);
	}
    }

    /** A test method which fills the block with a checkers pattern 
     * @param m the size of the blocks  */
    public void checkers(int m) {
	m = (1 << m);
	for(int i = 0; i < s.height; i++) {
	    for(int j = 0; j < s.width; j++) {
		set(i,j,(short)(((i&m)^(j&m))*255));
	    }
	}
    }

    /** a method to test for the equality of two blocks
     * Two blocks are found to be equal if each of their
     * points are equal */
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