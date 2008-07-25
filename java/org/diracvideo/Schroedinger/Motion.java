package org.diracvideo.Schroedinger;

/** Motion
 *
 * An ill-named class representing an object
 * which does motion compensation prediction
 * on a picture. **/
class Motion {
    Parameters par;
    Vector vecs[];
    Block temp;
    Picture refs[];
    Arithmetic ar[];

    public Motion(Parameters p, Picture r[]) {
	par = p;
	vecs = new Vector[par.x_num_blocks * par.y_num_blocks];
	temp = new Block(par.getBlockDimension());
	refs = r;
    }
    
    public void initialize(Buffer bufs[]) {
	ar = new Arithmetic[9];
	for(int i = 0; i < 9; i++) {
	    if(bufs[i] == null)
		continue;
	    ar[i] = new Arithmetic(bufs[i]);
	}
	for(int i = 0; i < vecs.length; i++) {
	    vecs[i] = new Vector();
	}
    }

    public void decodeMacroBlock(int x, int y) {
	int split = splitPrediction(x,y);
	Vector mv = getVector(x,y);
	mv.split = (split + ar[0].decodeUint(Context.SB_F1, Context.SB_DATA))%3;
	switch(mv.split) {
	case 0:
	    decodePredictionUnit(mv, x, y);
	    break;
	case 1:
	case 2:
	default:
	    break;
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


    
    public void render(Block frame) {
	
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
	return vecs[x + y*par.x_num_blocks];
    }
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