
/** Block
 *
 * Has methods for getting the correct positions
 * of elements in the data array. I would call it
 * Frame but that conflicts with java.awt.Frame. */


final class Block {
    public final short[] d;
    public final Point p;
    public final Dimension s, o;
    /** 
     * Default constructor of a Block.
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

    /**
     * @return the index for a general point in the frame **/
    public int index(int x, int y) {
	return line(y) + Util.clamp(x,0, s.width - 1);
    }
}