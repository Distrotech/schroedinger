package org.diracvideo.Schroedinger;
import java.awt.Dimension;
import java.awt.Image;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import javax.imageio.ImageIO;

public class Picture {
    private Buffer buf;
    private Wavelet wav;
    private Decoder dec;
    private VideoFormat format;
    private Parameters par;
    private int code;
    private Picture refs[];
    private boolean zero_residual = false;
    private SubBand[][] coeffs;
    private Block iwt_frame[], mc_frame[];
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
	if(!par.is_intra) {
	    refs = new Picture[par.num_refs];
	}
    }

    public Picture() {
	status = Decoder.Status.DONE;
	num = -1;
	img = new BufferedImage(200, 200, BufferedImage.TYPE_INT_RGB);
    }

    public synchronized void parse() {
	if(status != Decoder.Status.NULL)
	    return;
	status = Decoder.Status.WAIT;
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
	    for(int i = 0; i < refs.length; i++) {
		refs[i] = dec.refs.get(num + u.decodeSint());
		assert (refs[i] != null) : "Reference picture not found";
	    }
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
	if(!zero_residual) {
	    initializeIwtFrames();
	    decodeWaveletTransform();
	}
	if(!par.is_intra) {
	    decodeRefs();
	    initializeMCFrames();
	    decodeMotionCompensate();
	    if(true) {
		iwt_frame = mc_frame;
	    } else {
		for(int i = 0; i < 3; i++)
		    mc_frame[i].addTo(iwt_frame[i]); 
	    }
	}
	createImage();
	status = Decoder.Status.DONE;
    }

    
    private void decodeWaveletTransform() {
	for(int c = 0; c < 3; c++) {
	    short out[] = iwt_frame[c].d;
	    coeffs[c][0].decodeCoeffs(out);
	    coeffs[c][0].intraDCPredict(out);
	    for(int i = 0; i < par.transformDepth; i++) {
		coeffs[c][3*i+1].decodeCoeffs(out);
		coeffs[c][3*i+2].decodeCoeffs(out);
		coeffs[c][3*i+3].decodeCoeffs(out);
	    } 
	    wav.inverse(iwt_frame[c], par.transformDepth);  
	}
    }

    private void decodeRefs() {
	for(int i = 0; i < par.num_refs; i++) {
	    switch(refs[i].status) {
	    case DONE:
		break;
	    case NULL:
		refs[i].parse();
		refs[i].decode();
		i--;
		break;
	    case OK:
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
	motion.decode(motion_buffers);
	motion.render(mc_frame, format);
    }

    private void initializeIwtFrames() {
	iwt_frame = new Block[3];
	iwt_frame[0] = new Block(par.iwtLumaSize);
	iwt_frame[1] = new Block(par.iwtChromaSize);
	iwt_frame[2] = new Block(par.iwtChromaSize);
    }
    
    private void initializeMCFrames() {
	Dimension luma = new Dimension(format.width, format.height);
	Dimension chro = new Dimension(format.width >> format.chromaHShift(),
				       format.height >> format.chromaVShift());
	mc_frame = new Block[3];
	mc_frame[0] = new Block(luma);
	mc_frame[1] = new Block(chro);
	mc_frame[2] = new Block(chro);
    }

    private void decodeYUV(int pixels[]) {
	Dimension lum = par.iwtLumaSize;
	Dimension chrom = par.iwtChromaSize;
	ColourSpace col = format.colour;
	short y,u,v;
	short Y[] = iwt_frame[0].d, U[] = iwt_frame[1].d,
	    V[] = iwt_frame[2].d;
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

    public Block getComponent(int k) {
	return iwt_frame[k];
    }

    public void save(String streamname) {
	if(img == null ||
	   error != null) {
	    return;
	}
	try {
	    File f = new File(String.format("%s_%d.png", num));
	    ImageIO.write(img, "png", f);
	} catch(IOException x) {
	    System.err.format("Could not save pic %d for reason: %s\n",
			      num, x.getMessage());
	}
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
