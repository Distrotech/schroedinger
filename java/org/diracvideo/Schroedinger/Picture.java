package org.diracvideo.Schroedinger;
import java.awt.*;
import java.awt.image.*;
/* pictures in the dirac stream specification can basically parse themselves
   we are going to take advantage of that */

class SubBand {
    int qf, qo;
    Buffer buf;
    Parameters par;
    public SubBand (Buffer b, int q, Parameters p) {
	par = p;
	qf = quantFactor(q);
	qo = quantOffset(q);
	buf = b;
    }
    
    public void decodeCoeffs(short[] out, int s, int b, int e, int w) {
	/* basically the plan is to decode the coefficients
	   into the frame array right where they should be */
	if(buf != null) {
	    Unpack u = new Unpack(buf);
	    for(int i = b; i < e; i += w*s) {
		for(int j = i; j < w + i; j += s) {
		    out[j] = u.decodeSint(qf,qo);
		}
	    } 
	} 
    }

    private int quantFactor(int qi) {
	int base = (1 << (qi >>> 2));
	switch(qi & 0x3) {
	case 0:
	    return base << 2;
	case 1:
	    return (503829 * base + 52958)/105917;
	case 2:
	    return (665857 * base + 58854)/117708;
	case 3:
	default:
	    return (440253 * base + 32722)/65444;
	}
    }

    private int quantOffset(int qi) {
	if(qi == 0) {
	    return 0;
	} else {
	    if(par.num_refs == 0) {
		if(qi == 1) {
		    return 2;
		} else {
		    return (qf + 1) / 2;
		}
	    } else {
		return (qf * 3 + 4) / 8;
	    }
	}
    }
}

class Parameters {
    /* all that matters for now is wavelet depth */
    public int transform_depth = 4, wavelet_index;
    public int codeblock_mode_index = 0;
    public boolean no_ac, is_ref, is_lowdelay;
    public int[] horiz_codeblocks = new int[7],
	vert_codeblocks = new int[7];
    public int num_refs;

    public Parameters(int c) {
	no_ac = !((c & 0x48) == 0x8);
	num_refs = (c & 0x3);
	is_ref = (c & 0x0c) == 0x0c;
	is_lowdelay = ((c & 0x88) == 0x88);
    }

    public Dimension getIwtSize(VideoFormat format, boolean luma) {
	int size[] = {0,0};
	if(luma) {
	    format.getPictureLumaSize(size);
	} else {
	    format.getPictureChromaSize(size);
	}
	return new Dimension( Util.roundUpPow2(size[0], transform_depth),
			      Util.roundUpPow2(size[1], transform_depth));
    }

    public String toString() {
	StringBuilder sb = new StringBuilder();
	sb.append("\nParameters:\n");
	sb.append(String.format("Transform depth: %d\n", transform_depth));
	sb.append(String.format("Using ac: %c\n", (no_ac ? 'n' : 'y')));
	sb.append(String.format("Is ref: %c\n", (is_ref ? 'y' : 'n')));
	return sb.toString();
    }
}    


public class Picture {
    private Buffer buf;
    private Wavelet wav;
    private Decoder dec;
    private VideoFormat format;
    private Parameters par;
    private int code;
    private Picture[] refs = {null,null};
    private boolean zero_residual = false;
    private SubBand[][] coeffs;
    private short[][] frame;
    private BufferedImage img;
    public Decoder.Status status = Decoder.Status.OK;
    public Exception error = null;
    public final int num;


    /** Picture:
     * @c: picture parse code
     * @n: picture number
     * @b: payload buffer
     * @d: decoder of the picture 
     *
     * The b buffer should only point to the payload data section of 
     * the picture (not the header). The only methods that would ever need 
     * to be called are parse(), decode(), and getImage(). However,
     * one should check wether the error variable is set before and after
     * calling a method. One should not call them in any other order than that
     * just specified. Each can be called without arguments and should not be 
     * called twice. */

    public Picture(int c, int n, Buffer b, Decoder d) {
	num = n;
	code = c;
	buf = b;
	dec = d;
	format = d.getVideoFormat();
	par = new Parameters(c);
	coeffs = new SubBand[3][19];
    }

    public void parse() {
	try {
	    Unpack u = new Unpack(buf);
	    parseHeader(u);
	    if(par.num_refs > 0) {
		u.align();
		parsePredictionParameters(u);
		u.align();
		parseBlockData(u);
	    }
	    u.align();
	    if(par.num_refs > 0) {
		zero_residual = u.decodeBool();
	    }
	    if(!zero_residual) {
		parseTransformParameters(u);
		u.align();
		if(par.is_lowdelay) {
		    parseLowDelayTransformData(u);
		} else {
		    parseTransformData(u);
		}
	    }
	} catch(Exception e) {
	    error = e;
	    status = Decoder.Status.ERROR;
	}
	buf = null;
    }

    private void parseHeader(Unpack u) throws Exception {
	u.align();
	if(par.num_refs > 0) {
	    refs[0] = dec.refs.get(num + u.decodeSint());
	}
	if(par.num_refs > 1) {
	    refs[1] = dec.refs.get(num + u.decodeSint());
	}
	if(par.is_ref) {
	    int r = u.decodeSint();
	    if(r != 0) {
		dec.refs.remove(r + num);
	    }
	    dec.refs.add(this);
	}
    }
    
    private void parsePredictionParameters(Unpack u) throws Exception {
	System.err.println("parsePredictionParamters()");
    }

    private void parseBlockData(Unpack u) throws Exception {
	System.err.println("parseBlockData()");
    }

    private void parseTransformParameters(Unpack u) throws Exception {
	par.wavelet_index = u.decodeUint();
	par.transform_depth = u.decodeUint();
	if(!par.is_lowdelay) {
	    if(u.decodeBool()) {
		for(int i = 0; i < par.transform_depth + 1; i++) {
		    par.horiz_codeblocks[i] = u.decodeUint();
		    par.vert_codeblocks[i] = u.decodeUint();
		}
		par.codeblock_mode_index = u.decodeUint();
	    } else {
		for(int i = 0; i < par.transform_depth + 1; i++) {
		    par.horiz_codeblocks[i] = 1;
		    par.vert_codeblocks[i] = 1;
		}
	    }
	} else {
	    throw new Exception("Unhandled stream type");
	}
    }

    private void parseTransformData(Unpack u) throws Exception {
	for(int c = 0; c < 3; c++) {
	    for(int i = 0; i < 1+3*par.transform_depth; i++) {
		u.align();
		int l = u.decodeUint();
		if( l != 0) {
		    int q = u.decodeUint();
		    Buffer b = u.getSubBuffer(l);
		    coeffs[c][i] = new SubBand(b,q,par);
		} else {
		    coeffs[c][i] = new SubBand(null,0,par);
		}
	    }
	}
    }

    private void parseLowDelayTransformData(Unpack u) {
	System.err.println("parseLowDelayTransformData()");
    }

    public void decode() {
	decodeWaveletTransform();
	createImage();
    }

    
    private void decodeWaveletTransform() {
	initializeTransformFrames();
	decodeComponent(0);
	decodeComponent(1);
	decodeComponent(2);
    }

    private void decodeComponent(int c) {
	Dimension dim = par.getIwtSize(format, c == 0);
	int size = dim.width * dim.height;
	int stride = (1 << par.transform_depth);
	coeffs[c][0].decodeCoeffs(frame[c], stride,0, size, dim.width);
	for(int i = 1; i < par.transform_depth; i++) {
	    coeffs[c][3*i+1].decodeCoeffs(frame[c], stride,
					  stride >> 1, size, dim.width);
	    coeffs[c][3*i+2].decodeCoeffs(frame[c], stride, 
					  (stride * dim.width) >> 1,
					  size, dim.width);
	    coeffs[c][3*i+3].decodeCoeffs(frame[c], stride,
					  (stride * dim.width + stride) >> 1,
					  size, dim.width);
	    stride >>= 1;
	}
	Wavelet.inverse(frame[c], dim.width, par.transform_depth); 
    }

    private void initializeTransformFrames() {
	Dimension lum = par.getIwtSize(format, true);
	Dimension chrom = par.getIwtSize(format, true);
	frame = new short[3][];
	frame[0] = new short[lum.width * lum.height];
	frame[1] = new short[chrom.width * chrom.height];
	frame[2] = new short[chrom.width * chrom.height];
    }

    private void decodeYuv(int pixels[]) {
	Dimension dim = par.getIwtSize(format, true);
	for(int i = 0; i < format.height; i++) {
	    for(int j = 0; j < format.width; j++) {
		pixels[j + i*format.width] =
		    (0x1010100 *frame[0][j + i*dim.width]) >> 8;
	    }
	}
    }
    
    private void createImage() {
	img = new BufferedImage(format.width + 1, format.height + 1, 
				BufferedImage.TYPE_INT_RGB);
	WritableRaster ras = img.getRaster();
	int pixels[] = new int[format.width * format.height];
	decodeYuv(pixels);
	img.setRGB(0,0, format.width, format.height,pixels, 0,1);
    }

    public Image getImage() {
	if(img == null) {
	    createImage();
	}
	return img;
    }
    
    public String toString() {
	StringBuilder b = new StringBuilder();	   
	b.append(String.format("Picture number: %d with code %02X",
			       num, code));
	b.append(par.toString());
	if(status == Decoder.Status.OK) {
	    if(par.num_refs > 0) {
		b.append(String.format("\nHas %d reference(s)",
				       par.num_refs));
	    }
	    if(par.is_ref) {
		b.append("\nIs a reference");
	    }
	    for(int i = 0; i < par.num_refs; i++) {
		b.append(String.format("\n\treference[%d]: %d", 
				       i, refs[i].num));
	    }
	} else {
	    b.append(String.format("Picture ERROR: %s", error.toString()));
	}

	return b.toString();
    }

}
