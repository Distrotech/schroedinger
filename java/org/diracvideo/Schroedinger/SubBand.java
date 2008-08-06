package org.diracvideo.Schroedinger;
import java.awt.Dimension;

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

