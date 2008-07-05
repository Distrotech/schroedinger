package org.diracvideo.Schroedinger;
import java.awt.*;
import java.awt.image.*;
/* pictures in the dirac stream specification can basically parse themselves
   we are going to take advantage of that */

class SubBand {
    int quant,length, depth, width;
    Buffer buf;
    boolean luma;
    public SubBand (Buffer b, int q, int l) {
	quant = q;
	buf = b;
	length = l;
    }
    
    public void decodeCoefficients(short[] out, int s, int b, int e, int w) {
	/* basically the plan is to decode the coefficients
	   into the frame array right where they should be */
	Unpack u = new Unpack(buf);
	for(int i = b; i < e; i += w) {
	    for(int j = i; j - i < w; j += s) {
		out[j] = (short)u.decodeSint();
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
    private short[] frame;
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
		    coeffs[c][i] = new SubBand(b,q,l);
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
	Dimension dim = par.getIwtSize(format, true);
	int size = dim.width * dim.height;
	int stride = (1 << par.transform_depth);
	frame = new short[size];
	coeffs[0][0].decodeCoefficients(frame, stride,0, size, dim.width);
	/* this part should be in a loop */
	for(int i = 1; i < par.transform_depth; i++) {
	    coeffs[0][3*i+1].decodeCoefficients(frame, stride,
					    stride >> 1, size, dim.width);
	    coeffs[0][3*i+2].decodeCoefficients(frame, stride, 
						(stride * dim.width) >> 1,
						size, dim.width);
	    coeffs[0][3*i+3].decodeCoefficients(frame, stride,
						(stride * dim.width + stride) >> 1,
						size, dim.width);
	    stride >>= 1;
	}
	Wavelet.inverse(frame, dim.width, par.transform_depth); 
    }

    private void createImage() {
	img = new BufferedImage(format.width, format.height, 
				BufferedImage.TYPE_INT_RGB);
	Graphics gr = img.createGraphics();
	gr.drawString(String.format("Picture nr %d", num),20,20);
	gr.drawString(String.format("Dimensions: %d x %d", format.width,
				    format.height), 20,40);
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
