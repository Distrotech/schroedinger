package org.diracvideo.Schroedinger;

/** Buffer
 *
 * Buffer represents a one-dimensional array,
 */
final class Buffer {
    public final int b,e;
    public final byte[] d;

    public Buffer(byte d[], int b, int e) {
	this.b = Util.clamp(b,0,d.length);
	this.e = Util.clamp(e,b,d.length);
	this.d = d;
    }

    public Buffer(byte d[], int b) {
	this(d,b, d.length);
    }
    
    public Buffer(byte[] d) {
	this(d,0, d.length);
    }

    /** Get a subbuffer
     * 
     * Parameters are relative to the beginning of the block
     * 
     * @param b begin of block 
     * @param e end of block */

    public Buffer sub(int b, int e) {
	return new Buffer(this.d, this.b + b, this.b + e);
    }

    public Buffer sub(int b) {
	return new Buffer(this.d, this.b + b, this.e);
    }
    
    public byte getByte(int i) {
	return d[i + b];
    }

    public int getInt(int i) {
	int r = 0;
	for(int j = i + b; j < i + b + 4; j++)
	    r = (r << 8) | (d[j]&0xff);
	return r;
    }

    public int size() {
	return e - b;
    }

    public Buffer cat(Buffer o) {
	byte n[] = new byte[o.size() + size()];
	System.arraycopy(this.d, this.b, n, 0, e - b);
	System.arraycopy(o.d, o.b, n, e - b, o.e - o.b);
	return new Buffer(n);
    }
}
