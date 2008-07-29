package org.diracvideo.Schroedinger;

/** Motion
 *
 * An ill-named class representing an object
 * which does motion compensation prediction
 * on a picture. **/
class Motion {
    Parameters par;
    Vector vecs[];
    short tmp[];
    Picture refs[];
    Arithmetic ar[];
    int xbsep, ybsep, xblen, yblen, width, height;
    int xoffset, yoffset, max_fast_x, max_fast_y;
    short obmc[];
    Block block;
    
    public Motion(Parameters p, Picture r[]) {
	par = p;
	refs = r;
	vecs = new Vector[par.x_num_blocks * par.y_num_blocks];
	
	tmp = new short[64*64*3];
    }
    
    public void initialize(Buffer bufs[]) {
	ar = new Arithmetic[9];
	for(int i = 0; i < 9; i++) {
	    if(bufs[i] == null)
		continue;
	    ar[i] = new Arithmetic(bufs[i]);
	}
    }

    public void decodeMacroBlock(int x, int y) {
	int split = splitPrediction(x,y);
	Vector mv = getVector(x,y);
	mv.split = (split + ar[0].decodeUint(Context.SB_F1, Context.SB_DATA))%3;
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
	mv.pred_mode ^= ar[1].decodeBit(Context.BLOCK_MODE_REF1);
	if(par.num_refs > 1) {
	    mv.pred_mode ^= (ar[1].decodeBit(Context.BLOCK_MODE_REF2) << 1);
	}
	if(mv.pred_mode == 0) {
	    int pred[] = new int[3];
	    dcPrediction(x,y,pred);
	    mv.dc[0] = pred[0] + ar[6].decodeSint(Context.LUMA_DC_CONT_BIN1,
						  Context.LUMA_DC_VALUE,
						  Context.LUMA_DC_SIGN);
	    mv.dc[1] = pred[1] + ar[7].decodeSint(Context.CHROMA1_DC_CONT_BIN1,
						  Context.CHROMA1_DC_VALUE,
						  Context.CHROMA1_DC_SIGN);
	    mv.dc[2] = pred[2] + ar[8].decodeSint(Context.CHROMA2_DC_CONT_BIN1,
						  Context.CHROMA2_DC_VALUE,
						  Context.CHROMA2_DC_SIGN);
	} else {
	    int pred_x, pred_y;
	    if(par.have_global_motion) {
		int pred = globalPrediction(x,y);
		pred ^= ar[1].decodeBit(Context.GLOBAL_BLOCK);
		mv.using_global = (pred == 0 ? false : true);
	    } else {
		mv.using_global = false;
	    }
	    if(!mv.using_global) {
		if((mv.pred_mode & 1) != 0) {
		    vectorPrediction(mv,x,y,1);
		    mv.dx[0] += ar[2].decodeSint(Context.MV_REF1_H_CONT_BIN1,
						 Context.MV_REF1_H_VALUE, 
						 Context.MV_REF1_H_SIGN);
		    mv.dy[0] += ar[3].decodeSint(Context.MV_REF1_V_CONT_BIN1, 
						 Context.MV_REF1_V_VALUE,
						 Context.MV_REF1_V_SIGN);
		}
		if((mv.pred_mode & 2) != 0) {
		    vectorPrediction(mv, x, y, 2);
		    mv.dx[1] += ar[4].decodeSint(Context.MV_REF2_H_CONT_BIN1,
						 Context.MV_REF2_H_VALUE, 
						 Context.MV_REF2_H_SIGN);
		    mv.dy[1] += ar[5].decodeSint(Context.MV_REF2_V_CONT_BIN1, 
						 Context.MV_REF2_V_VALUE,
						 Context.MV_REF2_V_SIGN);

		}
	    } else {
		mv.dx[0] = 0;
		mv.dx[1] = 0;
		mv.dy[0] = 0;
		mv.dy[1] = 0;
	    }
	}
    }

    public void render(Block frame[], VideoFormat f) {
	for(int k = 0; k < frame.length; k++) {
	    if(k == 0) {
		xbsep = par.xbsep_luma;
		ybsep = par.ybsep_luma;
		xblen = par.xblen_luma;
		yblen = par.yblen_luma;
	    } else {
		xbsep = par.xbsep_luma >> f.chromaHShift();
		ybsep = par.ybsep_luma >> f.chromaVShift();
		xblen = par.xblen_luma >> f.chromaHShift();
		yblen = par.yblen_luma >> f.chromaVShift();
	    }
	    width = frame[k].s.width;
	    height = frame[k].s.height;
	    xoffset = (xblen - xbsep)/2;
	    yoffset = (xblen - ybsep)/2;
	    max_fast_x = (width - xblen) << par.mv_precision;
	    max_fast_y = (height - yblen) << par.mv_precision;
	    initOBMCWeight();
	    int max_x_blocks = Math.min(par.x_num_blocks - 1,
					(width - xoffset)/xbsep);
	    int max_y_blocks = Math.min(par.y_num_blocks - 1,
					(height - yoffset)/ybsep);
	    for(int i = 0; i < par.x_num_blocks; i++) {
		int x = xbsep * i - xoffset;
		predictBlock(x, -yoffset, k, i, 0);
		accumalateBlock(x, -yoffset, frame[k]);
	    }
	    for(int j = 1; j < max_y_blocks; j++) {
		
	    }
	}
    }

    private void predictBlock(int x, int y, int k, int i, int j) {
	Vector mv = getVector(i,j);
	switch(mv.pred_mode) {
	case 0:
	    getDCBlock(i, j, k, x, y);
	    break;
	case 1:
	    getRef1Block(i, j, k, x, y);
	    break;
	case 2:
	    getRef2Block(i, j, k, x, y);
	    break;
	case 3:
	    getBiRefBlock(i, j, k, x, y);
	    break;
	default:
	    break;
	}
    }

    private void accumalateBlock(int x, int y, Block frame) {

    }

    private void getDCBlock(int i, int j, int k, int x, int y) {
	
    }

    private void getRef1Block(int i, int j, int k, int x, int y) {
	
    }

    private void getRef2Block(int i, int j, int k, int x, int y) {
	
    }

    private void getBiRefBlock(int i, int j, int k, int x, int y) {
	
    }

    private void initOBMCWeight() {
	int wx, wy;
	int weight_y[] = new int[yblen],
	    weight_x[] = new int[xblen];
	obmc = new short[yblen*xblen];
	for(int i = 0; i < xblen; i++) {
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
	for(int j = 0; j < yblen; j++) {
	    for(int i = 0; i < xblen; i++) {
		obmc[i + j*xblen] = (short)(weight_x[i] * weight_y[j]);
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

/** Global
 *
 * The class for global motion estimation */
class Global {
    int b0, b1;
    int a_exp, a00, a01, a10, a11;
    int c_exp, c0, c1;
}


/** Vector
 *
 * The class representing a single motion vector
 * element.  */
class Vector {
    int weight, split, pred_mode;
    int dx[] = new int[2],
	dy[] = new int[2];
    int dc[] = new int[3];
    boolean using_global;
}