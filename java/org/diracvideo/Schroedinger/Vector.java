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
}