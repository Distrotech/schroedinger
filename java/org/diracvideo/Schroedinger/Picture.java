package org.diracvideo.Schroedinger;
import java.awt.Image;
/* pictures in the dirac stream specification can basically parse themselves
   we are going to take advantage of that */

public class Picture {
    private Buffer buf;
    private Wavelet wav;
    private Decoder dec;
    private VideoFormat format;
    private Parameters params;
    private int code;
    private Picture[] refs = {null,null};
    private Buffer[] coeffs;
    private boolean zero_residual = false;
    private Image img;
    public Decoder.Status status = Decoder.Status.OK;
    public Exception error = null;
    public final int num;



    private static class Parameters {
	/* all that matters for now is wavelet depth */
	public int iwt_chroma_size[] = {0,0},
	    iwt_luma_size[] = {0,0};
	public int transform_depth = 4, wavelet_index;
	public int codeblock_mode_index = 0;
	public boolean is_noarith, is_ref, is_lowdelay;
	public int[] horiz_codeblocks = new int[7],
	    vert_codeblocks = new int[7];
	public int num_refs;
	public Parameters(int c) {
	    is_noarith = (c & 0x48) == 0x8;
	    num_refs = (c & 0x3);
	    is_ref = (c & 0x0c) == 0x0c;
	    is_lowdelay = ((c & 0x88) == 0x88);
	}

	public void calculateIwtSizes(VideoFormat f) {
	    int size[] = {0,0};
	    f.getPictureLumaSize(size);
	    iwt_luma_size[0] = Util.roundUpPow2(size[0], transform_depth);
	    iwt_luma_size[1] = Util.roundUpPow2(size[1], transform_depth);
	    f.getPictureChromaSize(size);
	    iwt_chroma_size[0] = Util.roundUpPow2(size[0], transform_depth);
	    iwt_chroma_size[1] = Util.roundUpPow2(size[1], transform_depth);
	}


    }    

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
     * just specified. Each can be called without arguments and should not be called 
     * twice. */

    public Picture(int c, int n, Buffer b, Decoder d) {
	num = n;
	code = c;
	buf = b;
	dec = d;
	format = d.getVideoFormat();
	params = new Parameters(c);
	coeffs = new Buffer[13];
    }

    public void parse() {
	try {
	    Unpack u = new Unpack(buf);
	    parseHeader(u);
	    if(params.num_refs > 0) {
		u.align();
		parsePredictionParameters(u);
		u.align();
		parseBlockData(u);
	    }
	    u.align();
	    if(params.num_refs > 0) {
		zero_residual = u.decodeBool();
	    }
	    if(!zero_residual) {
		parseTransformParameters(u);
		params.calculateIwtSizes(format);
		u.align();
		if(params.is_lowdelay) {
		    parseLowDelayTransformData(u);
		} else {
		    parseTransformData(u);
		}
	    }
	} catch(Exception e) {
	    error = e;
	    status = Decoder.Status.ERROR;
	}
    }

    private void parseHeader(Unpack u) throws Exception {
	u.align();
	if(params.num_refs > 0) {
	    refs[0] = dec.refs.get(num + u.decodeSint());
	}
	if(params.num_refs > 1) {
	    refs[1] = dec.refs.get(num + u.decodeSint());
	}
	if(params.is_ref) {
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
	params.wavelet_index = u.decodeUint();
	params.transform_depth = u.decodeUint();
	if(!params.is_lowdelay) {
	    if(u.decodeBool()) {
		for(int i = 0; i < params.transform_depth + 1; i++) {
		    params.horiz_codeblocks[i] = u.decodeUint();
		    params.vert_codeblocks[i] = u.decodeUint();
		}
		params.codeblock_mode_index = u.decodeUint();
	    } else {
		for(int i = 0; i < params.transform_depth + 1; i++) {
		    params.horiz_codeblocks[i] = 1;
		    params.vert_codeblocks[i] = 1;
		}
	    }
	} else {
	    throw new Exception("Unhandled stream type");
	}
    }

    private void parseTransformData(Unpack u) throws Exception {
	for(int c = 0; c < 3; c++) {
	    for(int i = 0; i < 1+3*params.transform_depth; i++) {
		u.align();
		int l = u.decodeUint();
		if(l == 0) {
		    break;
		} else {
		    int q = u.decodeUint();
		    u.getSubBuffer(l);
		}
	    }
	}
    }

    private void parseLowDelayTransformData(Unpack u) {
	System.err.println("parseLowDelayTransformData()");
    }

    public void decode() {

    }

    public Image getImage() {
	return null;
    }
    
    public String toString() {
	StringBuilder b = new StringBuilder();	   
	b.append(String.format("Picture number: %d with code %02X",
			       num, code));
	if(status == Decoder.Status.OK) {
	    if(params.num_refs > 0) {
		b.append(String.format("\nHas %d reference(s)",
				       params.num_refs));
	    }
	    if(params.is_ref) {
		b.append("\nIs a reference");
	    }
	    for(int i = 0; i < params.num_refs; i++) {
		b.append(String.format("\n\treference[%d]: %d", 
				       i, refs[i].num));
	    }
	} else {
	    b.append(String.format("Picture ERROR: %s", error.toString()));
	}
	return b.toString();
    }

}
