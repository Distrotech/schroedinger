package org.diracvideo.Schroedinger;
import java.lang.Math;

public class Unpack {
    private byte d[];
    private int i = 0, r, l = 0, s;
    
    public Unpack(byte d[]) {
	this(d,0,d.length);
    }

    public Unpack(byte d[], int b, int e) {
	this.d = d;
	this.i = b;
	this.s = e;
	shiftIn();
    }
    
    public Unpack(Buffer b) {
	this(b.d, b.b, b.e);	
    }
	    
    private final void shiftIn() {
	int b = (s - i > 4 ? 3  : s - i - 1) * 8;
	for(; l <= b && i < s; l += 8) {
	    r |= (d[i++]&0xff) << (24-l);
	}
    }

    private final int shiftOut(int n) {
	int v;
	v = (r >>> (32 - n));
	l -= n;
	r <<= n;
	return v;
    }
    
    public void align() {
	r <<= (l & 7);
	l -= (l & 7);
	shiftIn();
    }

    public int decodeLit32() {
	int v;
	switch(l) {
	case 0:
	case 8:
	case 16:
	case 24:
	    shiftIn();
	case 32:
	    v = r;
	    r = 0;
	    l = 0;
	    return v;
	default:
	    return bits(32);
	}
    }

    public int bits(int n) {
	if (n > l) {
	    int t = l;
	    int v = shiftOut(t) << t;
	    shiftIn();
	    return v | shiftOut(n - t);
	} 
	return shiftOut(n);
    }

    public void skip(int n) { 
	if(n <= l) {
	    shiftOut(n);
	} else {
	    n -= l;
	    l = r = 0;
	    i += Math.min(s - i - 1, n >> 3);
	    if(i < s - 1 && (n & 7) != 0) {
		shiftIn();
		shiftOut(n & 7);
	    }
	}
	/*	/b\ werk videoplayer, werk!!!   */
    }
    
    public int decodeUint() {
	int v = 1;
	while(bits(1) == 0) {
	    v = (v << 1) | bits(1);
	}
	return v-1;
    }
    
    public int decodeSint() {
	int v = decodeUint();
	return (v == 0 || bits(1) == 0) ? v : -v;
    }

    public int bitsLeft() {
	return (s - i) * 8 + l;
    }

    public int bitsRead() {
	/* assuming, not generally true,
	   that i was zero at initialization */
	return i*8 - l;
    }
    
    public boolean decodeBool() {
	return bits(1) == 1;
    }

    public Buffer getSubBuffer(int bytes) {
	align();
	int start = i - l/8;
	Buffer buf = new Buffer(d, start, start + bytes);
	skip(bytes*8 + (l & 7));
	return buf;
    }
    
    public boolean equals(Unpack u) {
	boolean same = true;
	same = (same && u.bitsRead() == bitsRead());
	same = (same && u.bitsLeft() == bitsLeft());
	same = (same && u.d == d);
	same = (same && check() && u.check());
	return same;
    }

    public boolean check() {
	int t = 0;
	if (l == 0) {
	    return r == 0;
	}
	for(int j = -4; j < 0; j++) {
	    t = (t << 8) | (d[i+j]&0xff);
	}
	t <<= (32 - l);
	return t == r;
    }

    public Unpack clone() {
	Unpack n = new Unpack(d);
	n.i = this.i;
	n.s = this.s;
	n.l = this.l;
	n.r = this.r;
	return n;
    }

    public String toString() {
	StringBuilder b = new StringBuilder();
	b.append(String.format("Register: %08X\n", r));
	b.append(String.format("Bits left: %d\tIndex: %d", l,i));
	return b.toString();
    }
}

