package org.diracvideo.Schroedinger;

/** 
 * Decoder
 *
 * An interface to decoding a dirac stream. 
 * Most (all) of the actual work is done by the
 * Picture class, however Decoder can do general
 * dispatching, scheduling and bookkeeping.
 * That is the reason we keep it arround */

public class Decoder {
    private VideoFormat format;
    private Picture fault;
    private int next_frame_number;
    public Status status = Status.OK;
    private Buffer next;
    public Exception e;
    public Queue refs, in, out;
    public enum Status {NULL, OK, WAIT, DONE, ERROR}

    public Decoder() {
	next_frame_number = 0;
	refs = new Queue(4);
	in = new Queue(4);
	out = new Queue(4);
	fault = new Picture();
    }
     
    /** Push:
     * @param d byte array containing stream data
     * @param o offset in the byte array
     * 
     * The stated goal of this function is that
     * it should work even in the case of a so-called
     * DumbMuxingFormat which splits the dirac stream
     * into x-byte segments, and that the driver program
     * should only ever have to push such segments to 
     * the decoder. */

    public synchronized void push(byte d[], int o, int l) throws Exception {
	push(new Buffer(d,o));
    }

    private void push(Buffer buf) throws Exception {
	// HOI BIE...NIET TEVEEL MOPPEREN VANDAAG HE! :)
	if(in.full()) { /* we can't do anything */
	    if(next == null)
		next = buf;
	    else 
		next = next.cat(buf);
	    return;
	}
	if(next == null) { /* no previous packet */
	    if(buf.size() < 13) { /* no complete header */
		next = buf;
		return;
	    }
	    if(buf.getInt(0) != 0x42424344)
		throw new Exception("Incorrect Magic Code (no Dirac stream)");
	    int n = buf.getInt(5);
	    if(buf.size() < n) {
		next = buf;
		return;
	    }
	    dispatchBuffer(buf.sub(0,n)); /* add complete packet */
	    if(buf.size() > n)
		push(buf.sub(n));  /* push rest */
	} else {
	    if(next.size() < 13) { /* again, no complete header */
		buf = next.cat(buf);
		next = null;
		push(buf); /* try again with current packet added */
		return;
	    } 
	    if(next.getInt(0) != 0x42424344) 
		throw new Exception("Incorret Magic Code (no Dirac stream)");
	    int n = next.getInt(5);
	    if(next.size() >= n) { /* complete packet in next buffer */
		dispatchBuffer(next.sub(0,n));
		if(next.size() > n)
		    buf = next.sub(n).cat(buf); 
		next = null;
		if(buf.size() > 0)
		    push(buf);
		return;
	    }
	    if(next.size() + buf.size() >= n) { 
		/* complete packet in buffers together */
		int copy = n - next.size();
		next = next.cat(buf.sub(0, copy));
		dispatchBuffer(next);
		next = null;
		if(buf.size() > copy)
		    push(buf.sub(copy));
		return;
	    }
	    /* incomplete packet in both buffers */
	    next = next.cat(buf);
	}
	
    }


    /* at this point, the buffer must be a complete dirac packet */
    private void dispatchBuffer(Buffer b) throws Exception {
	assert (b.getInt(5) == b.size()) : "Incorrect buffer sizes";
	byte c = b.getByte(4);
	switch(c) {
	case 0x00:
	    VideoFormat tmp = new VideoFormat(b);
	    if(format == null) {
		format = tmp;
	    } else if(!tmp.equals(format)) {
		throw new Exception("Stream Error: Inequal Video Formats");
	    }
	    break;
	case 0x10:
	    status = Status.DONE;
	    break;
	case 0x20:
	case 0x30:
	    break;
	default:
	    if(format == null) {
		throw new Exception("Stream Error: Picture Before Header");
	    }
	    Picture pic = new Picture(b, this);
	    in.add(pic);
	    break;
	}
    }

    public synchronized void reset(int next_frame) {

    }

    public synchronized Picture pull() {
	Picture p = out.get(next_frame_number);
	if(p != null) 
	    out.remove(next_frame_number++);
	return p;
    }

    /** A decoding loop */
    public void run() {
	while(!out.full() && !in.empty()) {
	    try {
		Picture pic = in.pop();
		if(pic.status == Status.NULL) { 
		    pic.parse();
//		    WAS JE EVEN NAAR DE WC??? :)
		    pic.decode();
		}
		if(pic.status == Status.DONE) {
		    out.add(pic);
		}
	    } catch(Throwable t) {
		t.printStackTrace();
	    }
	}
    }

    /** getVideoFormat
     *
     * Returns the in-use videoformat object. An application 
     * can use it to determine the size of the window for the
     * video and other such useful properties. This would be 
     * useless if format where public, which is a 
     * reasonable option. 
     * @see VideoFormat */

    public VideoFormat getVideoFormat() {
	return format;
   }

    public boolean done() {
	return status == Status.DONE;
    }

    public boolean full() {
	return in.full();
    }
}

