package org.diracvideo.Schroedinger;




public class Decoder {
    private VideoFormat format;
    private int next_frame_number;
    private int major_version, minor_version, profile, level;
    public Status status = Status.OK;
    public Exception e;
    public Queue refs, out;
    public enum Status {OK, WAIT, DONE, ERROR}

    public Decoder() {
	next_frame_number = 0;
	refs = new Queue(4);
	out = new Queue(100);
    }

    /** Push:
     * @d: array containing data from the stream
     *
     * Pushes an array of stream data onto the decoder.
     * Currently each packet should be pushed seperately.
     * Pushing incomplete packets could result in parse errors */

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
	} /* If we come here, the buffer should be a picture.
	     Thus we should throw an error if there is no
	     video format */
	if(format == null) {
	    throw new Exception("No Video Format");
	}
	int n = u.decodeLit32();
	Picture p = new Picture(c,n,new Buffer(d,17), this);
	p.parse();
	p.decode();
	if(p.error == null) {
	    out.add(p);
	}
    }

    public synchronized Picture pull() {
	try {
	    Picture p = out.get(next_frame_number);
	    out.remove(next_frame_number++);
	    return p;
	} catch(Exception e) {
	    return new Picture(0x48,next_frame_number++,null,this);
	}
    }

    public boolean hasPicture() {
	return out.has(next_frame_number);
    }
    
    public void run() {
	
    }

    /** getVideoFormat:
     *
     * Returns the in-use videoformat object. An application 
     * can use it to determine the size of the window for the
     * video and other such useful properties */
    public VideoFormat getVideoFormat() {
	return format;
    }
}

