package org.diracvideo.Schroedinger;
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