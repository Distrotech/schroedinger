package org.diracvideo.Schroedinger;
import java.awt.Dimension;
import java.awt.Point;

/** Motion
 *
 * An ill-named class representing an object
 * which does motion compensation prediction
 * on a picture. **/
class Motion {
    Parameters par;
    Vector vecs[];
    Picture refs[];
    Arithmetic ar[];
    int xbsep, ybsep, xblen, yblen, xoffset, yoffset;
    int chroma_h_shift, chroma_v_shift;
    short weight_x[], weight_y[];
    Block block, tmp_ref[];

    static int ARITH_SUPERBLOCK = 0;
    static int ARITH_PRED_MODE = 1;
    static int ARITH_REF1_X = 2;
    static int ARITH_REF1_Y = 3;
    static int ARITH_REF2_X = 4;
    static int ARITH_REF2_Y = 5;
    static int ARITH_DC_0 = 6;
    static int ARITH_DC_1 = 7;
    static int ARITH_DC_2 = 8;

    public Motion(Parameters p, Picture r[]) {
	par = p;
	refs = r;
	vecs = new Vector[par.x_num_blocks * par.y_num_blocks];
	tmp_ref = new Block[refs.length];
    }
    
    private void initialize(Buffer bufs[]) {
	ar = new Arithmetic[9];
	for(int i = 0; i < 9; i++) {
	    if(bufs[i] == null)
		continue;
	    ar[i] = new Arithmetic(bufs[i]);
	}
    }

    public void decode(Buffer bufs[]) {
	initialize(bufs);
	for(int y = 0; y < par.y_num_blocks; y += 4)
	    for(int x = 0; x < par.x_num_blocks; x += 4)
		decodeMacroBlock(x,y);	
    }

    private void decodeMacroBlock(int x, int y) {
	int split = splitPrediction(x,y);
	Vector mv = getVector(x,y);
	mv.split = (split + ar[ARITH_SUPERBLOCK].decodeUint(Context.SB_F1, Context.SB_DATA))%3;
	switch(mv.split) {
	case 0:
	    decodePredictionUnit(mv, x, y);
	    for(int i = 0; i < 4; i++) 
		for(int j = 0; j < 4; j++) 
		    setVector(mv, x + j, y + i);
	    break;
	case 1:
	    for(int i = 0; i < 4; i += 2) 
		for(int j = 0; j < 4; j += 2) {
		    mv = getVector(x + j, y + i);
		    mv.split = 1;
		    decodePredictionUnit(mv, x + j, y + i);
		    setVector(mv, x + j + 1, y + i);
		    setVector(mv, x + j, y + i + 1);
		    setVector(mv, x + j + 1, y + i + 1);
		}
	    break;
	case 2:
	    for(int i = 0; i < 4; i++) 
		for(int j = 0; j < 4; j++) {
		    mv = getVector(x + j, y + i);
		    mv.split = 2;
		    decodePredictionUnit(mv, x + j, y + i);
		}
	    break;
	default:
	    throw new Error("Unsupported splitting mode");
	}
    }

    private void decodePredictionUnit(Vector mv, int x, int y) {
	mv.pred_mode = modePrediction(x,y);
	mv.pred_mode ^= ar[ARITH_PRED_MODE].decodeBit(Context.BLOCK_MODE_REF1);
	if(par.num_refs > 1) {
	    mv.pred_mode ^= (ar[ARITH_PRED_MODE].decodeBit(Context.BLOCK_MODE_REF2) << 1);
	}
	if(mv.pred_mode == 0) {
	    int pred[] = new int[3];
	    dcPrediction(x,y,pred);
	    mv.dc[0] = pred[0] + 
		ar[ARITH_DC_0].decodeSint(Context.LUMA_DC_CONT_BIN1,
					  Context.LUMA_DC_VALUE,
					  Context.LUMA_DC_SIGN);
	    mv.dc[1] = pred[1] + 
		ar[ARITH_DC_1].decodeSint(Context.CHROMA1_DC_CONT_BIN1,
					  Context.CHROMA1_DC_VALUE,
					  Context.CHROMA1_DC_SIGN);
	    mv.dc[2] = pred[2] + 
		ar[ARITH_DC_2].decodeSint(Context.CHROMA2_DC_CONT_BIN1,
					  Context.CHROMA2_DC_VALUE,
					  Context.CHROMA2_DC_SIGN);
	} else {
	    int pred_x, pred_y;
	    if(par.have_global_motion) {
		int pred = globalPrediction(x,y);
		pred ^= ar[ARITH_SUPERBLOCK].decodeBit(Context.GLOBAL_BLOCK);
		mv.using_global = (pred == 0 ? false : true);
	    } else {
		mv.using_global = false;
	    }
	    if(!mv.using_global) {
		if((mv.pred_mode & 1) != 0) {
		    vectorPrediction(mv,x,y,1);
		    mv.dx[0] += 
			ar[ARITH_REF1_X].decodeSint(Context.MV_REF1_H_CONT_BIN1,
						    Context.MV_REF1_H_VALUE, 
						    Context.MV_REF1_H_SIGN);
		    mv.dy[0] +=
			ar[ARITH_REF1_Y].decodeSint(Context.MV_REF1_V_CONT_BIN1, 
						    Context.MV_REF1_V_VALUE,
						    Context.MV_REF1_V_SIGN);
		}
		if((mv.pred_mode & 2) != 0) {
		    vectorPrediction(mv, x, y, 2);
		    mv.dx[1] += ar[ARITH_REF2_X].decodeSint(Context.MV_REF2_H_CONT_BIN1,
							    Context.MV_REF2_H_VALUE, 
							    Context.MV_REF2_H_SIGN);
		    mv.dy[1] += ar[ARITH_REF2_Y].decodeSint(Context.MV_REF2_V_CONT_BIN1, 
						 Context.MV_REF2_V_VALUE,
						 Context.MV_REF2_V_SIGN);

		}
	    } 
	}
	mv.dx[0] = 0;
	mv.dy[0] = 0;
	mv.dx[1] = 0;
	mv.dy[1] = 0;
    }

    public void render(Block out[], VideoFormat f) {
	for(int k = 0; k < out.length; k++) {
	    initializeRender(k,f);
	    block = new Block(new Dimension(xblen, yblen));
	    for(int i = 0; i < par.num_refs; i++) {
		tmp_ref[i] = refs[i].getComponent(k);
		if(par.mv_precision > 0)
		    tmp_ref[i] = tmp_ref[i].upSample();
	    }
	    for(int j = 0; j < par.y_num_blocks; j++)
		for(int i = 0; i < par.x_num_blocks; i++) {
		    predictBlock(out[k], i, j, k);
		    accumulateBlock(out[k], i*xbsep - xoffset, 
				    j*ybsep - yoffset);
		}
	    out[k].shiftOut(6,32);
	    out[k].clip(7);
	}
    }

    private void initializeRender(int k, VideoFormat f) {
	chroma_h_shift = f.chromaHShift();
	chroma_v_shift = f.chromaVShift();
	yblen = par.yblen_luma;
	xblen = par.xblen_luma;
	ybsep = par.ybsep_luma;
	xbsep = par.xbsep_luma;
	if(k != 0) {
	    yblen >>= chroma_v_shift;
	    ybsep >>= chroma_v_shift;
	    xbsep >>= chroma_h_shift;
	    xblen >>= chroma_h_shift;
	}
	yoffset = (yblen - ybsep) >> 1;
	xoffset = (xblen - xbsep) >> 1;
	/* initialize obmc weight */
	weight_y = new short[yblen*2];
	weight_x = new short[xblen*2];
	for(int i = 0; i < xblen; i++) {
	    short wx;
	    if(xoffset == 0) {
		wx = 8;
	    } else if( i < 2*xoffset) {
		wx = Util.getRamp(i, xoffset);
	    } else if(xblen - 1 - i < 2*xoffset) {
		wx = Util.getRamp(xblen - 1 - i, xoffset);
	    } else {
		wx = 8;
	    }
	    weight_x[i] = wx;
	}
	for(int j = 0; j < yblen; j++) {
	    short wy;
	    if(yoffset == 0) {
		wy = 8;
	    } else if(j < 2*yoffset) {
		wy = Util.getRamp(j, yoffset);
	    } else if(yblen - 1 - j < 2*yoffset) {
		wy = Util.getRamp(yblen - 1 - j, yoffset);
	    } else {
		wy = 8;
	    }
	    weight_y[j] = wy;
	}
    }

    private void predictBlock(Block out, int i, int j, int k) {
	int xstart = (i*xbsep) - xoffset, 
	    ystart = (j*ybsep) - yoffset;
	Vector mv = getVector(i,j);
	if(mv.pred_mode == 0) {
	    for(int q = 0; j < yblen; j++) 
		for(int p = 0; i < xblen; i++)
		    block.set(p, q, (mv.dc[k]));
	} 
	if(k != 0 && !mv.using_global)
	    mv = mv.scale(chroma_h_shift, chroma_v_shift); 
	for(int q = 0; q < yblen; q++) {
	    int y = ystart + q;
	    if(y < 0 || y > out.s.height - 1) continue;
	    for(int p = 0; p < xblen; p++) {
		int x = xstart + p;
		if(x < 0 || x > out.s.width - 1) continue;
		block.set(p,q, predictPixel(mv, x, y, k));
	    }
	}
    }


    private short predictPixel(Vector mv, int x,  int y, int k) {
	if(mv.using_global) {
	    for(int i = 0; i < par.num_refs; i++) {
		par.global[i].getVector(mv, x, y, i);
	    }
	    if(k != 0) 
		mv = mv.scale(chroma_h_shift, chroma_v_shift);
	}
	short weight = (short)(par.picture_weight_1 + par.picture_weight_2);
	short val = 0;
	int px, py;
	switch(mv.pred_mode) {
	case 1:
	    px = (x << par.mv_precision) + mv.dx[0];
	    py = (y << par.mv_precision) + mv.dy[0];
	    val = (short)(weight*predictSubPixel(0, px, py));
	    break;
	case 2:
	    px = (x << par.mv_precision) + mv.dx[1];
	    py = (y << par.mv_precision) + mv.dy[1];
	    val = (short)(weight*predictSubPixel(1, px, py));
	    break;
	case 3:
	    px = (x << par.mv_precision) + mv.dx[0];
	    py = (y << par.mv_precision) + mv.dy[0];
	    val = (short)(par.picture_weight_1*predictSubPixel(0, px, py));
	    px = (x << par.mv_precision) + mv.dx[1];
	    py = (x << par.mv_precision) + mv.dy[1];
	    val += (short)(par.picture_weight_2*predictSubPixel(1, px, py));
	default:
	    break;
	}
	return (short)Util.roundShift(val, par.picture_weight_bits);
    }

    private short predictSubPixel(int ref, int px, int py) {
	if(par.mv_precision < 2) { 
	    return tmp_ref[ref].real(px, py); 
	}
	int prec = par.mv_precision;
	int add = 1 << (prec - 1);
	int hx = px >> (prec-1);
	int hy = py >> (prec-1);
	int rx = px - (hx << (prec-1));
	int ry = py - (hy << (prec-1));
	int w00,w01, w10, w11;
	w00 = (add - rx)*(add - ry);
	w01 = (add - rx)*ry;
	w10 = rx*(add - ry);
	w11 = rx*ry;
	int val = w00*tmp_ref[ref].real(hx, hy) + 
	    w01*tmp_ref[ref].real(hx + 1, hy) +
	    w10*tmp_ref[ref].real(hx, hy + 1) + 
	    w11*tmp_ref[ref].real(hx + 1, hy + 1);
	return (short)((val + (1 << (2*prec-3))) >> (2*prec - 2));
    }
    

    private void accumulateBlock(Block out, int x, int y) {
	for(int q = 0; q < yblen; q++) {
	    if(q + y < 0 || q + y >= out.s.height) continue;
	    for(int p = 0; p < xblen; p++) {
		if(p + x < 0 || p + x >= out.s.width) continue;
		int weight = weight_x[p]*weight_y[q];
		int val = out.pixel(p + x, q + y) + block.pixel(p,q);
		out.set(p + x, q + y, val * weight);
	    }
	}
    }

    private int splitPrediction(int x, int y) {
	if(y == 0) {
	    if(x == 0) {
		return 0;
	    } else {
		return vecs[x-4].split;
	    }
	} else {
	    if(x == 0) {
		return getVector(0, y - 4).split;
	    } else {
		int sum = 0;
		sum += getVector(x, y - 4).split;
		sum += getVector(x - 4, y).split;
		sum += getVector(x - 4, y - 4).split;
		return (sum+1)/3;
	    }
	}
    }

    private int modePrediction(int x, int y) {
	if(y == 0) {
	    if(x == 0) {
		return 0;
	    } else {
		return vecs[x - 1].pred_mode;
	    }
	} else {
	    if(x == 0) {
		return getVector(0, y - 1).pred_mode;
	    } else {
		int a,b,c;
		a = getVector(x - 1, y).pred_mode;
		b = getVector(x, y - 1).pred_mode;
		c = getVector(x - 1, y - 1).pred_mode;
		return (a&b)|(b&c)|(c&a);
	    }
	}
    }

    private int globalPrediction(int x, int y) {
	if(x == 0 && y == 0) {
	    return 0;
	}
	if(y == 0) {
	    return vecs[x-1].using_global ? 1 : 0;
	}
	if(x == 0) {
	    return getVector(0, y-1).using_global ? 1 : 0;
	}
	int sum = 0;
	sum += getVector(x - 1, y).using_global ? 1 : 0;
	sum += getVector(x, y - 1).using_global ? 1 : 0;
	sum += getVector(x - 1, y - 1).using_global ? 1 : 0;
	return (sum >= 2) ? 1 : 0;
    }
    
    private void vectorPrediction(Vector mv, int x, int y, int mode) {
	int n = 0, vx[] = new int[3], vy[] = new int[3];
	if(x > 0) {
	    Vector ov = getVector(x-1, y);
	    if(!ov.using_global && 
	       (ov.pred_mode & mode) != 0) {
		vx[n] = ov.dx[mode-1];
		vy[n] = ov.dx[mode-1];
		n++;
	    }
	}
	if(y > 0) {
	    Vector ov = getVector(x, y-1);
	    if(!ov.using_global &&
	       (ov.pred_mode & mode) != 0) {
		vx[n] = ov.dx[mode-1];
		vy[n] = ov.dx[mode-1];
		n++;
	    }
	}
	if(x > 0 && y > 0) {
	    Vector ov = getVector(x - 1, y - 1);
	    if(!ov.using_global &&
	       (ov.pred_mode & mode) != 0) {
		vx[n] = ov.dx[mode-1];
		vy[n] = ov.dy[mode-1];
		n++;
	    }
	}
	switch(n) {
	case 0:
	    mv.dx[mode-1] = 0;
	    mv.dy[mode-1] = 0;
	    break;
	case 1:
	    mv.dx[mode-1] = vx[0];
	    mv.dy[mode-1] = vy[0];
	    break;
	case 2:	  
	    mv.dx[mode-1] = (vx[0] + vx[1] + 1) >> 1;
	    mv.dy[mode-1] = (vy[0] + vy[1] + 1) >> 1;
	    break;
	case 3:
	    mv.dx[mode-1] = Util.median(vx);
	    mv.dy[mode-1] = Util.median(vy);
	    break;
	}
    }

    private void dcPrediction(int x, int y, int pred[]) {
	for(int i = 0; i < 3; i++) {
	    int sum = 0, n = 0;
	    if(x > 0) {
		Vector ov = getVector(x - 1, y);
		if(ov.pred_mode == 0) {
		    sum += ov.dc[i];
		    n++;
		}
	    }
	    if(y > 0) {
		Vector ov = getVector(x, y - 1);
		if(ov.pred_mode == 0) {
		    sum += ov.dc[i];
		    n++;
		}
	    }
	    if(x > 0 && y > 0) {
		Vector ov = getVector(x - 1, y - 1);
		if(ov.pred_mode == 0) {
		    sum += ov.dc[i];
		    n++;
		}
	    }
	    switch(n) {
	    case 0:
		pred[i] = 0;
		break;
	    case 1:
		pred[i] = sum;
		break;
	    case 2:
		pred[i] = (sum+1)>>1;
		break;
	    case 3:
		pred[i] = (sum+1)/3;
		break;
	    }
	}
    }

    private final Vector getVector(int x, int y) {
	int pos = x + y*par.x_num_blocks;
	if(vecs[pos] == null) {
	    vecs[pos] = new Vector();
	}
	return vecs[pos];
    }

    private final void setVector(Vector mv, int x, int y) {
	vecs[x + y*par.x_num_blocks] = mv;
    }

}

