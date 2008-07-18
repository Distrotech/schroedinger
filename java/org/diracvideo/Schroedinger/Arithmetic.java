package org.diracvideo.Schroedinger;

class Context {
    int prob, next;
    public Context(int n, int p) {
	next = n;
	prob = p;
    }
    public static int ZERO = 0;
}

public class Arithmetic {
    private int offset, size, code, low, range, cntr;
    private byte shift;
    private byte[] data;
    private short probabilities[];
    private Context contexts[];

    /* static lookup tables */
    static short lut[] = {

    };

    static short next_list[] = {

    };

    public Arithmetic(Buffer b) {
	data = b.d;
	offset = b.b;
	size = b.e;
	decodeInit();
    }

    private void decodeInit() {
	low = 0;
	range = 0xffff;
	probabilities = new short[70];
	contexts = new Context[70];
	for(int i = 0; i < contexts.length; i++) {
	    contexts[i] = new Context(Context.ZERO, 0x8000);
	}
    }

    public int decodeUint(int cont, int val) {
	int v = 1;
	while(decodeBool(cont)) {
	    cont = contexts[cont].next;
	    v = (v << 1) | decodeBit(val);
	}
	return v-1;
    }

    public int decodeSint(int cont, int val, int sign) {
	int v = decodeUint(cont, val);
	return (v == 0 || decodeBool(sign)) ? v : -v;
    }

    public int decodeBit(int context) {
	return (decodeBool(context) ? 1 : 0);
    }

    public boolean decodeBool(int context) {
	boolean v;
	int range_times_prob, lut_index;
	range_times_prob =
	    (range * probabilities[context]) >> 16;
	lut_index = probabilities[context] >> 8;
	v = (code - low >= range_times_prob);
	probabilities[context] += 
	    Arithmetic.lut[(v ? (1 << 8) : 0) | lut_index];
	if(v) {
	    low += range_times_prob;
	    range = -range_times_prob;
	} else {
	    range = range_times_prob;
	}

	while(range <= 0x4000) {
	    low <<= 1;
	    range <<= 1;
	    code <<= 1;
	    code |= (shift >> (7-cntr))&1;
	    cntr++;
	    if(cntr == 8) {
		offset++;
		if(offset < size) {
		    shift = data[offset];
		} else {
		    shift = (byte)0xff;
		}
		low &= 0xffff;
		code &= 0xffff;
		if(code < low) {
		    code |= (1<<16);
		}
		cntr = 0;
	    }
	}
	return v;
    }
}