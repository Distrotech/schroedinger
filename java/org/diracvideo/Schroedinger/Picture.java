package org.diracvideo.Schroedinger;
import java.awt.*;
import java.awt.image.*;

class SubBand {
    private int qi, level, stride, offset, orient, numX, numY;
    private Buffer buf;
    private Dimension frame, band;
    private Parameters par;

    public SubBand (Buffer b, int q, Parameters p) {
	par = p;
	qi = q;
	buf = b;
    }
    
    public void calculateSizes(int i, boolean luma) {
	level = (i-1)/3;
	int shift = (par.transformDepth - level);
	frame = luma ? par.iwtLumaSize : par.iwtChromaSize;
	orient = (i - 1) % 3 + 1;
	stride = (1 << shift);
	offset = (orient == 0 ? 0 : 
		  (orient == 1 ? stride >> 1 :
		   (orient == 2 ? (frame.width * stride) >> 1 :
		    (stride + frame.width * stride) >> 1)));
	numX = (orient == 0 ? par.horiz_codeblocks[0] :
		par.horiz_codeblocks[level+1]);
	numY = (orient == 0 ? par.vert_codeblocks[0] :
		par.vert_codeblocks[level+1]);
	band = new Dimension(frame.width >> shift, frame.height >> shift);
    }

    /* Maybe we should rewrite this using blocks.
     * I'm not sure */
    public void decodeCoeffs(short[] out) {
	if(buf == null)
	    return;
	int bounds[] = {0,0,0};
	if(par.no_ac) {
	    Unpack u = new Unpack(buf);
	    if(numX * numY == 1) {	
		bounds[1] = out.length;
		bounds[2] = frame.width;
		decodeCodeBlock(out,u,bounds);
		return;
	    }
	    for(int y = 0; y < numY; y++) {
		for(int x = 0; x < numX; x++) {
		    if(u.decodeBool())
			continue;
		    if(par.codeblock_mode_index != 0)
			qi += u.decodeSint();
		    calculateBounds(bounds,x,y);
		    decodeCodeBlock(out,u,bounds);
		}
	    } 
	} else {
	    Arithmetic a = new Arithmetic(buf);
	    if(numX * numY == 1) {
		bounds[1] = out.length;
		bounds[2] = frame.width;
		decodeCodeBlock(out, a, bounds);
		return;
	    }
	    for(int y = 0; y < numY; y++) {
		for(int x = 0; x < numX; x++) {
		    if(a.decodeBool(Context.ZERO_CODEBLOCK))
			continue;
		    if(par.codeblock_mode_index != 0)
			qi += a.decodeSint(Context.QUANTISER_CONT,
					   Context.QUANTISER_VALUE,
					   Context.QUANTISER_SIGN);
		    calculateBounds(bounds, x, y);
		    decodeCodeBlock(out,a,bounds);
		}
	    } 
	}
    }


    private void decodeCodeBlock(short[] out, Unpack u,
				 int bounds[]) {
	int qo = quantOffset(qi);
	int qf = quantFactor(qi);
	for(int i = bounds[0] + offset; i < bounds[1];
	    i += frame.width * stride) {
	    for(int j = i; j < i + bounds[2]; j += stride) {
		out[j] = u.decodeSint(qf,qo);
	    }
	}
    }

    private void calculateBounds(int bounds[], int blockX, int blockY) {
	int shift = par.transformDepth - level;
	int startX = ((band.width * blockX)/numX) << shift;
	int startY = ((band.height * blockY)/numY) << shift;
	bounds[0] = (frame.width * startY) + startX;
	int endX = ((band.width * (blockX+1))/numX) << shift;
	int endY = ((band.height * (blockY+1))/numY) << shift;
	bounds[1] = ((endY-1)*frame.width) + endX;
	bounds[2] = endX - startX;
    }

    private void decodeCodeBlock(short[] out, Arithmetic a, int bounds[]) {
	int qo = quantOffset(qi);
	int qf = quantFactor(qi);
	for(int i = bounds[0]+offset; i < bounds[1]; i += frame.width*stride) {
	    decodeLineGeneric(out, a, i, bounds[2], qf, qo);
	}
    }

    private void decodeLineGeneric(short out[], Arithmetic a, int lineOffset,
				   int blockWidth, int qf, int qo) {
	int y = (lineOffset - offset)/(stride*frame.width);
	int x = ((lineOffset)%(frame.width))/stride;
	int parentLine = (y/2)*(2*frame.width*stride);
	int parentOffset = (orient == 1 ? stride :
			    (orient == 2 ? stride*frame.width : 
			     stride*frame.width + stride));
	for(int i = lineOffset; i < blockWidth + lineOffset; i += stride) {
	    boolean zparent = true, znhood = true;
	    if(level > 0) {
		zparent = (out[parentLine + parentOffset+(x/2)*stride*2] == 0);
	    } 
	    if(x > 0) znhood = (znhood && out[i-stride] == 0);
	    if(y > 0) znhood = (znhood && out[i-stride*frame.width] == 0);
	    if(x > 0 && y > 0) 
		znhood = (znhood && out[i-stride-stride*frame.width] == 0);
	    int cont = 0, sign = 0;
	    if(zparent) {
		cont = (znhood ? Context.ZPZN_F1 : Context.ZPNN_F1);
	    } else {
		cont = (znhood ? Context.NPZN_F1 : Context.NPNN_F1);
	    }
	    int v = a.decodeUint(cont, Context.COEFF_DATA);
	    if(orient == 1 && y > 0) {
		sign = out[i - frame.width*stride];
	    } else if(orient == 2 && x > 0) {
		sign = out[i - stride];
	    }
	    sign = (sign > 0 ? Context.SIGN_POS : 
		    (sign < 0 ? Context.SIGN_NEG : Context.SIGN_ZERO));
	    if(v > 0) {
		v = (v * qf + qo + 2) >> 2;
		v = (a.decodeBool(sign) ? -v : v);
	    } 
	    out[i] = (short)v;
	    x++;
	}
    }

    public void intraDCPredict(short out[]) {
	int predict = 0;
	for(int i = offset; i < out.length; i += frame.width * stride) {
	    for(int j = i; j < i + frame.width; j += stride) {
		if(j > i && i > 0) {
		    predict = Util.mean(out[j - stride], 
					out[j - frame.width*stride],
					out[j - frame.width*stride - stride]);
		} else if(j > i && i == 0) {
		    predict = out[j-stride];
		} else if(j == i && i > 0) {
		    predict = out[j-stride*frame.width];
		} else {
		    predict = 0;
		}
		out[j] += predict;
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
	    if(par.is_intra) {
		if(qi == 1) {
		    return 2;
		} else {
		    return (quantFactor(qi) + 1) / 2;
		}
	    } else {
		return (quantFactor(qi) * 3 + 4) / 8;
	    }
	}
    }
}


class Parameters {
    /* static array of wavelets */ 
    private static Wavelet[] wavs = {
	new DeslauriesDebuc(),
	new LeGall5_3(),
	new Wavelet()
    };
    
    /* Wavelet transform parameters */
    public int transformDepth = 4, wavelet_index;
    public int codeblock_mode_index = 0;
    public boolean no_ac, is_ref, is_lowdelay, is_intra;
    public int[] horiz_codeblocks = new int[7],
	vert_codeblocks = new int[7];
    public int num_refs;
    public Dimension iwtLumaSize, iwtChromaSize;
    /* Motion prediction parametrs */
    public int xblen_luma, yblen_luma, 
	xbsep_luma, ybsep_luma;
    public int x_num_blocks, y_num_blocks,
	x_offset, y_offset;
    public boolean have_global_motion;
    public int picture_prediction_mode, mv_precision;
    public int picture_weight_bits = 1, 
	picture_weight_1 = 1,picture_weight_2 = 1;
    public Global global[] = new Global[3];

    public Parameters(int c) {
	no_ac = !((c & 0x48) == 0x8);
	num_refs = (c & 0x3);
	is_ref = (c & 0x0c) == 0x0c;
	is_lowdelay = ((c & 0x88) == 0x88);
	is_intra = (num_refs == 0);
    }

    public void calculateIwtSizes(VideoFormat format) {
	int size[] = {0,0};
	format.getPictureLumaSize(size);
	iwtLumaSize = new Dimension(Util.roundUpPow2(size[0], transformDepth),
				    Util.roundUpPow2(size[1], transformDepth));
	format.getPictureChromaSize(size);
	iwtChromaSize = new Dimension(Util.roundUpPow2(size[0], transformDepth),
				      Util.roundUpPow2(size[1], transformDepth));
    }

    public void verifyBlockParams() throws Exception {
	boolean ok = true;
	ok = ok && xblen_luma >= 0;
	ok = ok && yblen_luma >= 0;
	ok = ok && xbsep_luma >= 0;
	ok = ok && ybsep_luma >= 0;
	if(!ok) {
	    throw new Exception("Block Paramters incorrect");
	}
    }

    public void setBlockParams(int i)
	throws Exception {
	switch(i) {
	case 1:
	    xblen_luma = yblen_luma = 8;
	    xbsep_luma = ybsep_luma = 4;
	    break;
	case 2:
	    xblen_luma = yblen_luma = 12;
	    xbsep_luma = ybsep_luma = 8;
	    break;
	case 3:
	    xblen_luma = yblen_luma = 16;
	    xbsep_luma = ybsep_luma = 12;
	    break;
	case 4:
	    xblen_luma = yblen_luma = 24;
	    xbsep_luma = ybsep_luma = 16;
	    break;
	default:
	    throw new Exception("Unsupported Block Parameters index");
	}

    }

    public void calculateMCSizes() {
	x_num_blocks = 4*Util.divideRoundUp(iwtLumaSize.width, 4*xbsep_luma);
	y_num_blocks = 4*Util.divideRoundUp(iwtLumaSize.height, 4*ybsep_luma);
	x_offset = (xblen_luma - xbsep_luma)/2;
	y_offset = (yblen_luma - ybsep_luma)/2;
    }
    
    public Wavelet getWavelet() {
	return wavs[wavelet_index];
    }

    

    public String toString() {
	StringBuilder sb = new StringBuilder();
	sb.append("\nParameters:\n");
	sb.append(String.format("Transform depth: %d\n", transformDepth));
	sb.append(String.format("Using ac: %c\n", (no_ac ? 'n' : 'y')));
	sb.append(String.format("Is ref: %c", (is_ref ? 'y' : 'n')));
	for(int i = 0; i < transformDepth; i++) {
	    sb.append(String.format("\nHorizBlocks: %d\tVertBlocks: %d",
				    horiz_codeblocks[i], vert_codeblocks[i]));
	}
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
    private Block[] frame;
    private Motion motion;
    private BufferedImage img;
    private Buffer[] motion_buffers;
    public Decoder.Status status;
    public Exception error = null;
    public final int num;


    /** Picture:
     * @param b payload buffer
     * @param d decoder of the picture 
     *
     * The b buffer should only point to the payload data section of 
     * the picture (not the header). The only methods that would ever need 
     * to be called are parse(), decode(), and getImage(). However,
     * one should check wether the error variable is set before and after
     * calling a method. One should not call them in any other order than that
     * just specified. Each can be called without arguments and should not be 
     * called twice. */

    public Picture(Buffer b, Decoder d) {
	num = b.getInt(13);
	code = b.getByte(4);
	buf = b;
	dec = d;
	format = d.getVideoFormat();
	par = new Parameters(code);
	coeffs = new SubBand[3][19];
	motion_buffers = new Buffer[9];
	status = Decoder.Status.NULL;
    }

    public Picture() { num = 0;  }

    public synchronized void parse() {
	if(status != Decoder.Status.NULL)
	    return;
	try {
	    Unpack u = new Unpack(buf);
	    u.skip(136); /* 17 * 8 */
	    parseHeader(u);
	    par.calculateIwtSizes(format);
	    if(!par.is_intra) {
		u.align();
		parsePredictionParameters(u);
		par.calculateMCSizes();
		u.align();
		parseBlockData(u);
	    }
	    u.align();
	    if(!par.is_intra) {
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
	    status = Decoder.Status.OK;
	} catch(Exception e) {
	    error = e;
	    status = Decoder.Status.ERROR;
	}
	buf = null;
    }

    private void parseHeader(Unpack u) throws Exception {
	u.align();
	if(!par.is_intra) {
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
	int index = u.decodeUint();
	if(index == 0) {
	    par.xblen_luma = u.decodeUint();
	    par.yblen_luma = u.decodeUint();
	    par.xbsep_luma = u.decodeUint();
	    par.ybsep_luma = u.decodeUint();
	    par.verifyBlockParams();
	} else {
	    par.setBlockParams(index);
	}
	par.mv_precision = u.decodeUint();
	if(par.mv_precision > 3) {
	    throw new Exception("mv_precision greater than supported");
	}
	par.have_global_motion = u.decodeBool();
	if(par.have_global_motion) {
	    for(int i = 0; i < par.num_refs; i++) {
		Global gm = par.global[i];
		if(u.decodeBool()) {
		    gm.b0 = u.decodeSint();
		    gm.b1 = u.decodeSint();
		} else {
		    gm.b0 = gm.b1 = 0;
		}
		if(u.decodeBool()) {
		    gm.a_exp = u.decodeUint();
		    gm.a00 = u.decodeSint();
		    gm.a01 = u.decodeSint();
		    gm.a10 = u.decodeSint();
		    gm.a11 = u.decodeSint();
		} else {
		    gm.a_exp = 0;
		    gm.a00 = gm.a11 = 1;
		    gm.a10 = gm.a01 = 0;
		}
		if(u.decodeBool()) {
		    gm.c_exp = u.decodeUint();
		    gm.c0 = u.decodeSint();
		    gm.c1 = u.decodeSint();
		} else {
		    gm.c_exp = gm.c0 = gm.c1 = 0;
		}
	    }
	}
	par.picture_prediction_mode = u.decodeUint();
	if(par.picture_prediction_mode != 0) {
	    throw new Exception("Unsupported picture prediction mode");
	}
	if(u.decodeBool()) {
	    par.picture_weight_bits = u.decodeUint();
	    par.picture_weight_1 = u.decodeSint();
	    if(par.num_refs > 1) {
		par.picture_weight_2 = u.decodeSint();
	    }
	}
    }

    private void parseBlockData(Unpack u) throws Exception {
	for(int i = 0; i < 9; i++) {
	    if(par.num_refs < 2 && (i == 4 || i == 5))
		continue;
	    int l = u.decodeUint();
	    motion_buffers[i] = u.getSubBuffer(l);
	}
    }

    private void parseTransformParameters(Unpack u) throws Exception {
	par.wavelet_index = u.decodeUint();
	wav = par.getWavelet();
	par.transformDepth = u.decodeUint();
	if(!par.is_lowdelay) {
	    if(u.decodeBool()) {
		for(int i = 0; i < par.transformDepth + 1; i++) {
		    par.horiz_codeblocks[i] = u.decodeUint();
		    par.vert_codeblocks[i] = u.decodeUint();
		}
		par.codeblock_mode_index = u.decodeUint();
	    } else {
		for(int i = 0; i < par.transformDepth + 1; i++) {
		    par.horiz_codeblocks[i] = 1;
		    par.vert_codeblocks[i] = 1;
		}
	    }
	} else {
	    throw new Exception("Unhandled stream type");
	}
    }

    private void parseTransformData(Unpack u) throws Exception {
	int q,l;
	Buffer b;
	for(int c = 0; c < 3; c++) {
	    for(int i = 0; i < 1+3*par.transformDepth; i++) {
		u.align();
		l = u.decodeUint();
		if( l != 0) {
		    q = u.decodeUint();
		    b = u.getSubBuffer(l);
		} else {
		    q = 0;
		    b = null;
		}
		coeffs[c][i] = new SubBand(b,q,par);
		coeffs[c][i].calculateSizes(i, c == 0);
	    }
	}
    }


    private void parseLowDelayTransformData(Unpack u) {
	System.err.println("parseLowDelayTransformData()");
    }

    /** synchronized decoding
     *
     * Decodes the picture. */
    public synchronized void decode() {
	if(status != Decoder.Status.OK)
	    return;
	status = Decoder.Status.WAIT;
	initializeFrames();
	if(!zero_residual) {
	    decodeWaveletTransform();
	}
	if(!par.is_intra) {
	    decodeRefs();
	    decodeMotionCompensate();
	}
	createImage();
	status = Decoder.Status.DONE;
    }

    
    private void decodeWaveletTransform() {
	for(int c = 0; c < 3; c++) {
	    short out[] = frame[c].d;
	    coeffs[c][0].decodeCoeffs(out);
	    coeffs[c][0].intraDCPredict(out);
	    for(int i = 0; i < par.transformDepth; i++) {
		coeffs[c][3*i+1].decodeCoeffs(out);
		coeffs[c][3*i+2].decodeCoeffs(out);
		coeffs[c][3*i+3].decodeCoeffs(out);
	    } 
	    wav.inverse(frame[c], par.transformDepth);  
	}
    }

    private void decodeRefs() {
	for(int i = 0; i < par.num_refs; i++) {
	    switch(refs[i].status) {
	    case DONE:
		break;
	    case NULL:
		refs[i].parse();
	    case OK:
		refs[i].decode();
		i--;
		break;
	    case WAIT:
		synchronized(refs[i]) {} /* wait for the decoding to end */
		break;
	    case ERROR:
		error = refs[i].error;
		status = Decoder.Status.ERROR;
		break;
	    }
	}
    }

    private void decodeMotionCompensate() {
	motion = new Motion(par, refs);
	motion.initialize(motion_buffers);
	for(int y = 0; y < par.y_num_blocks; y += 4)
	    for(int x = 0; x < par.x_num_blocks; x += 4)
		motion.decodeMacroBlock(x,y);
	motion.render(frame, format);
    }

    public short getPixel(int x, int y, int k) {
	return frame[k].pixel(x,y);
    }

    private void initializeFrames() {
	frame = new Block[3];
	frame[0] = new Block(par.iwtLumaSize);
	frame[1] = new Block(par.iwtChromaSize);
	frame[2] = new Block(par.iwtChromaSize);
    }

    private void decodeYUV(int pixels[]) {
	Dimension lum = par.iwtLumaSize;
	Dimension chrom = par.iwtChromaSize;
	ColourSpace col = format.colour;
	short y,u,v;
	short Y[] = frame[0].d, U[] = frame[1].d, V[] = frame[2].d;
	int xFac = (lum.width > chrom.width ? 2 : 1);
	int yFac = (lum.height > chrom.height ? 2 : 1);
        for(int i = 0; i < format.height; i++) {
            for(int j = 0; j < format.width; j++) {
		y = (short)(Y[j + i*lum.width]+128);
		u = (short)(U[j/xFac + (i/yFac)*chrom.width]);
		v = (short)(V[j/xFac + (i/yFac)*chrom.width]);
                pixels[j + i*format.width] = col.convert(y,u,v);
            }
	} 

    }
    
    private void createImage() {
	img = new BufferedImage(format.width , format.height , 
				BufferedImage.TYPE_INT_RGB);
	if(error != null) {
	    return;
	}
	int pixels[] = new int[format.width * format.height];
	decodeYUV(pixels);
	img.setRGB(0,0, format.width, format.height, pixels, 0, format.width);
    }

    /** getImage returns the displayable image of the picture
     *
     * Returns a black image when error != null. Does no work
     * when there already is an image created - can be called
     * multiple times. **/
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
	    if(!par.is_intra) {
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
