package org.diracvideo.Schroedinger;

/* pictures in the dirac stream specification can basically parse themselves
   we are going to take advantage of that */

public class Picture {
    private Buffer buf;
    private WaveletTransform wav;
    private Decoder dec;
    private VideoFormat format;
    private Parameters params;
    private int code;
    private Picture[] refs = {null,null};
    public Decoder.Status status = Decoder.Status.OK;
    public Exception error = null;
    public final int num;



    private static class Parameters {
	/* all that matters for now is wavelet depth */
	public int iwt_chroma_size[] = {0,0},
	    iwt_luma_size[] = {0,0};
	public int transform_depth = 4;
	public boolean is_noarith, is_ref;
	public int num_refs;
	public Parameters(int c) {
	    is_noarith = (c & 0x48) == 0x8;
	    num_refs = (c & 0x3);
	    is_ref = (c & 0x0c) == 0x0c;
	}

	public void calculateIwtSize(VideoFormat f) {
	    int size[] = {0,0};
	    f.getPictureLumaSize(size);
	    iwt_luma_size[0] = Util.roundUpPow2(size[0], transform_depth);
	    iwt_luma_size[1] = Util.roundUpPow2(size[1], transform_depth);
	    f.getPictureChromaSize(size);
	    iwt_chroma_size[0] = Util.roundUpPow2(size[0], transform_depth);
	    iwt_chroma_size[1] = Util.roundUpPow2(size[1], transform_depth);
	}
    }    

    public Picture(int c, int n, Buffer b, Decoder d) {
	num = n;
	code = c;
	buf = b;
	dec = d;
	format = d.getVideoFormat();
	params = new Parameters(c);

    }

    public void parse() {
	Unpack u = new Unpack(buf);
	parseHeader(u);
    }

    private void parseHeader(Unpack u) {
	u.align();
	try {
	    if(params.num_refs > 0) {
		refs[0] = dec.getReference(num + u.decodeSint());
	    }
	    if(params.num_refs > 1) {
		refs[1] = dec.getReference(num + u.decodeSint());
	    }
	    if(params.is_ref) {
		int r = u.decodeSint();
		if(r != 0) {
		    dec.retire(r + num);
		}
		dec.addReference(this);
	    }
	} catch(Exception e) {
	    error = e;
	    status = Decoder.Status.ERROR;
	}
    }
    
    public void decode() {

    }
    
    public String toString() {
	StringBuilder b = new StringBuilder();	   
	b.append(String.format("Picture number: %d with code %02X\n",
			       num, code));
	if(status == Decoder.Status.OK) {
	    if(params.num_refs > 0) {
		b.append(String.format("Has %d reference(s)\n",
				       params.num_refs));
	    }
	    if(params.is_ref) {
		b.append("Is a reference\n");
	    }
	    for(int i = 0; i < params.num_refs; i++) {
		b.append(String.format("\treference[%d]: %d\n", 
				       i, refs[i].num));
	    }
	} else {
	    b.append(String.format("Picture ERROR: %s", error.getMessage()));
	}
	return b.toString();
    }

}
