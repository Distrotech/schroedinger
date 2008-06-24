package org.diracvideo.Schroedinger;



public class Decoder {
    private VideoFormat format;
    private int next_frame_number;
    private int major_version, minor_version, profile, level;
    private Status status = Status.OK;
    private Exception e;
    public Queue refs, in, out;

    public enum Status {OK, WAIT, DONE, ERROR}

    public Decoder() {
	next_frame_number = 0;
	refs = new Queue(4);
	in = new Queue(4);
	out = new Queue(4);
    }
    
    public void push(byte d[]) throws Exception {
	Unpack u = new Unpack(d);
	int v = u.decodeLit32();
	if(v != 0x42424344) {
	    throw new Exception("Cannot handle stream");
	}
	int c = u.bits(8);
	u.decodeLit32();
	u.decodeLit32();
	if (0x00 == c) {
	    major_version = u.decodeUint();
	    minor_version = u.decodeUint();
	    profile = u.decodeUint();
	    level = u.decodeUint();
	    VideoFormat f = new VideoFormat(u);
	    if(this.format != null && 
	       !f.equals(format)) {
		throw new Exception("Different video formats");
	    }
	    format = f;
	    return;
	} else if (0x10 == c) {
	    status = Status.DONE;
	    return;
	} else if (0x20 == c || 0x30 == c) {
	    return;
	}
	int n = u.decodeLit32();
	Picture p = new Picture(c,n,new Buffer(d,17), this);
	p.parse();
	in.add(p);
	System.err.println(p);
    }

    public Picture pull() {
	try {
	    Picture p = out.get(next_frame_number);
	    out.remove(next_frame_number++);
	    return p;
	} catch(Exception e) {
	    return null;
	}
    }
    
    public void run() {
	
    }
    
    public VideoFormat getVideoFormat() {
	return format;
    }
}

