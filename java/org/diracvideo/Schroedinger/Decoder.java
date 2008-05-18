package org.diracvideo.Schroedinger;



public class Decoder {
    private VideoFormat format;
    private boolean eos = false;
    private int next_frame_number;


    public Decoder() {
	next_frame_number = 0;
    }
    
    public void push(byte d[]) throws Exception {
	Unpack u = new Unpack(d);
	int v = u.bits(32);
	if(v != 0x42424344) {
	    throw new Exception("Cannot handle stream");
	}
	int c = u.bits(8);
	u.bits(32);
	u.bits(32);
	if (0x00 == c) {
	    VideoFormat f = new VideoFormat(new Buffer(d,13));
	    if(this.format != null && 
	       !f.equals(format)) {
		throw new Error("Differing video formats");
	    }
	    format = f;
	    return;
	} else if (0x10 == c) {
	    eos = true;
	    return;
	} else if (0x20 == c || 0x30 == c) {
	    return;
	}
	int n = u.bits(32);
	Picture p = new Picture(c, n, new Buffer(d,17), format);
	p.parse();
	/* in.add(p); */
    }

    public void pull() {

    }
    
    public void run() {
	/* for each picture:
	   p.decode();
	   out.add(p); */
    }

}

