package org.diracvideo.Schroedinger;

public class Arithmetic {
    private int offset, size, code, low, range, cntr;
    private byte shift, data[];
    private int probabilities[];
    private Context contexts[];
    private short lut[];
    /* static lookup tables */
    static short LUT[] = {
	0,    2,    5,    8,   11,   15,   20,   24,
	29,   35,   41,   47,   53,   60,   67,   74,
	82,   89,   97,  106,  114,  123,  132,  141,
	150,  160,  170,  180,  190,  201,  211,  222,
	233,  244,  256,  267,  279,  291,  303,  315,
	327,  340,  353,  366,  379,  392,  405,  419,
	433,  447,  461,  475,  489,  504,  518,  533,
	548,  563,  578,  593,  609,  624,  640,  656,
	672,  688,  705,  721,  738,  754,  771,  788,
	805,  822,  840,  857,  875,  892,  910,  928,
	946,  964,  983, 1001, 1020, 1038, 1057, 1076,
	1095, 1114, 1133, 1153, 1172, 1192, 1211, 1231,
	1251, 1271, 1291, 1311, 1332, 1352, 1373, 1393,
	1414, 1435, 1456, 1477, 1498, 1520, 1541, 1562,
	1584, 1606, 1628, 1649, 1671, 1694, 1716, 1738,
	1760, 1783, 1806, 1828, 1851, 1874, 1897, 1920,
	1935, 1942, 1949, 1955, 1961, 1968, 1974, 1980,
	1985, 1991, 1996, 2001, 2006, 2011, 2016, 2021,
	2025, 2029, 2033, 2037, 2040, 2044, 2047, 2050,
	2053, 2056, 2058, 2061, 2063, 2065, 2066, 2068,
	2069, 2070, 2071, 2072, 2072, 2072, 2072, 2072,
	2072, 2071, 2070, 2069, 2068, 2066, 2065, 2063,
	2060, 2058, 2055, 2052, 2049, 2045, 2042, 2038,
	2033, 2029, 2024, 2019, 2013, 2008, 2002, 1996,
	1989, 1982, 1975, 1968, 1960, 1952, 1943, 1934,
	1925, 1916, 1906, 1896, 1885, 1874, 1863, 1851,
	1839, 1827, 1814, 1800, 1786, 1772, 1757, 1742,
	1727, 1710, 1694, 1676, 1659, 1640, 1622, 1602,
	1582, 1561, 1540, 1518, 1495, 1471, 1447, 1422,
	1396, 1369, 1341, 1312, 1282, 1251, 1219, 1186,
	1151, 1114, 1077, 1037,  995,  952,  906,  857,
	805,  750,  690,  625,  553,  471,  376,  255
    };

    static int next_list[] = {
	0, Context.QUANTISER_CONT,0,0,
	Context.ZP_F2, Context.ZP_F2,
	Context.ZP_F3, Context.ZP_F4,
	Context.ZP_F5, Context.ZP_F6p,
	Context.ZP_F6p,	Context.NP_F2,
	Context.NP_F2, Context.NP_F3,
	Context.NP_F4, Context.NP_F5,
	Context.NP_F6p,	Context.NP_F6p,
	0, 0, 0, 0,Context.SB_F2,
	Context.SB_F2, 0, 0, 0,	0,
	Context.LUMA_DC_CONT_BIN2,
	Context.LUMA_DC_CONT_BIN2,
	0, 0, Context.CHROMA1_DC_CONT_BIN2,
	Context.CHROMA1_DC_CONT_BIN2,
	0, 0, Context.CHROMA2_DC_CONT_BIN2,
	Context.CHROMA2_DC_CONT_BIN2,
	0, 0, Context.MV_REF1_H_CONT_BIN2,
	Context.MV_REF1_H_CONT_BIN3,
	Context.MV_REF1_H_CONT_BIN4,
	Context.MV_REF1_H_CONT_BIN5,
	Context.MV_REF1_H_CONT_BIN5,
	0, 0, Context.MV_REF1_V_CONT_BIN2,
	Context.MV_REF1_V_CONT_BIN3,
	Context.MV_REF1_V_CONT_BIN4,
	Context.MV_REF1_V_CONT_BIN5,
	Context.MV_REF1_V_CONT_BIN5,
	0, 0, Context.MV_REF2_H_CONT_BIN2,
	Context.MV_REF2_H_CONT_BIN3,
	Context.MV_REF2_H_CONT_BIN4,
	Context.MV_REF2_H_CONT_BIN5,
	Context.MV_REF2_H_CONT_BIN5,
	0, 0, Context.MV_REF2_V_CONT_BIN2,
	Context.MV_REF2_V_CONT_BIN3,
	Context.MV_REF2_V_CONT_BIN4,
	Context.MV_REF2_V_CONT_BIN5,
	Context.MV_REF2_V_CONT_BIN5,
	0, 0, 0
    };

    public Arithmetic(Buffer b) {
	data = b.d;
	offset = b.b;
	size = b.e;
	decodeInit();
    }

    public Arithmetic(byte d[]) {
	this(new Buffer(d));
    }

    private void decodeInit() {
	cntr = 0;
	low = 0;
	range = 0xffff;
	if(size - offset > 1) {
	    code = ((data[offset]&0xff)<<8) | (data[offset+1]&0xff);
		} else if(size - offset > 0) {
	    code = ((data[offset]&0xff)<<8) | 0xff;
	} else {
	    code = 0xffff;
	}
	offset += 2;
	shift = (size - offset > 0 ? data[offset] : (byte)0xff);
	probabilities = new int[Context.LAST];
	contexts = new Context[Context.LAST];
	lut = new short[512];
	for(int i = 0; i < contexts.length; i++) {
	    contexts[i] = new Context(next_list[i], 0x8000);
	    probabilities[i] = 0x8000;
	}
	for(int i = 0; i < 256; i++) {
	    lut[i] = Arithmetic.LUT[255-i];
	    lut[256+i] = (short)(-Arithmetic.LUT[i]);
	}
    }

    public int decodeUint(int cont, int val) {
	int v = 1;
	while(!decodeBool(cont)) {
	    cont = contexts[cont].next;
	    v = (v << 1) | decodeBit(val);
	}
	return v-1;
    }

    public int decodeSint(int cont, int val, int sign) {
	int v = decodeUint(cont, val);
	return (v == 0 || decodeBool(sign)) ? -v : v;
    }

    public int decodeBit(int context) {
	return (decodeBool(context) ? 1 : 0);
    }

    public boolean decodeBool(int context) {
	boolean v;
	int range_times_prob, lut_index;
	range_times_prob =
	    (range * probabilities[context]) >> 16;
	v = (code - low >= range_times_prob);
	lut_index = probabilities[context] >> 8 | (v ? 256 : 0);;
	probabilities[context] += lut[lut_index];
	if(v) {
	    low += range_times_prob;
	    range -= range_times_prob;
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

    /* an estimate of how many bits there are left */
    public int bytesLeft() { 
	return (size - offset);
    }
}