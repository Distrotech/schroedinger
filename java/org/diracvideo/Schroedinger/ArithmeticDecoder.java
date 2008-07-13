package org.diracvideo.Schroedinger;

class ArithmeticContext {
    int next, stat_range, n_bits, n_symbols, ones;
}

class ArithmeticDecoder {
    int code, range_size, cntr;
    byte shift;
    short probabilities[];
    int range[];
    int index, offset, carry;;
    ArithmeticContext contexts[];

    public ArithmeticDecoder(Buffer b) {
	range =  new int[2];
	range[0] = 0;
	range[1] = 0xffff;
	range_size = 0xffff;
	code = 0;
	
    }
}