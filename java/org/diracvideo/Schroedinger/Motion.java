package org.diracvideo.Schroedinger;

/** Motion
 *
 * An ill-named class representing an object
 * which does motion compensation prediction
 * on a picture. **/
class Motion {
    Vector vecs[];
    short temp[];
    Picture refs[];

    public Motion() {

    }
    
    public void render(Block frame) {

    }

}

/** Vector
 *
 * The class representing a single motion vector
 * element.  */
class Vector {
    int weight, dx, dy;
    
}