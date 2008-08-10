package org.diracvideo.Schroedinger;
/** Vector
 *
 * The class representing a single motion vector
 * element.  */

class Vector {
    int split, pred_mode;
    int dx[] = new int[2],
	dy[] = new int[2];
    int dc[] = new int[3];
    boolean using_global;

    public Vector scale(int h_shift, int v_shift) {
	Vector o = new Vector();
	o.using_global = using_global;
	o.pred_mode = pred_mode;
	o.dx[0] = dx[0] >> h_shift;
	o.dx[1] = dx[1] >> h_shift;
	o.dy[0] = dy[0] >> v_shift;
	o.dy[1] = dy[1] >> v_shift;
	return o;
    }

    public String toString() {
	StringBuilder sb = new StringBuilder();
	if(!using_global) {
	    switch(pred_mode) {
	    case 0:
		sb.append("Intra Vector\n");
		for(int i = 0; i < 3; i++)
		    sb.append(String.format("DC[%d] = %d\n", i, dc[i]));
		break;
	    case 1:
		sb.append("Ref1 Vector\n");
		sb.append(String.format("dx[0] = %d\n", dx[0]));
		sb.append(String.format("dy[0] = %d\n", dy[0]));
		break;
	    case 2:
		sb.append("Ref2 Vector\n");
		sb.append(String.format("dx[1] = %d\n", dx[1]));
		sb.append(String.format("dy[1] = %d\n", dy[1]));
		break;
	    case 3:
		sb.append("Ref1And2Vector\n");
		for(int i = 0; i < 2; i++) {
		    sb.append(String.format("dx[%d] = %d\n", i, dx[i]));	
		    sb.append(String.format("dy[%d] = %d\n", i, dy[i])); 
		}
		break;
	    }

	} else {
	    sb.append("Global Vector\n");
	}
	return sb.toString();
    }
}