import org.diracvideo.Schroedinger.*;

public class Wavelet2dTest {
    public static void main(String a[]) {
	if(false) {
	    synthesizeTest();
	    interLeaveTest();
	    twoDimensionTest();
	}
    }

    private static void synthesizeTest() {
	short d[] = {0,0,0,0,0,0,0,0,8,1};
	short c[] = {0,1,2,3,4,5,6,7,8,9};
	Wavelet.synthesize(d,4,0,d.length);
	Wavelet.synthesize(d,2,0,d.length);
	Wavelet.synthesize(d,1,0,d.length);
	for(int i = 0; i < d.length; i++) {
	    if(c[i] != d[i]) {
		throw new Error("Wavelet Error: synthesize");
	    }
	}
    }

    private static void interLeaveTest() {
	short ll[] = new short[36];
	short lh[] = new short[36];	
	short hl[] = new short[36];
	short hh[] = new short[36];
	fill(ll,1);
	fill(lh,2);
	fill(hl,3);
	fill(hh,4);
	short frame[] = Wavelet.interleave(ll,lh,hl,hh,6);
//	System.err.println(printable(frame,12));
    }

    private static void twoDimensionTest() {
	short ll[] = new short[36];
	short other[] = new short[36];
	fill(ll,1);
	short frame[] = Wavelet.interleave(ll, other, other, other, 6);
	Wavelet.inverse(frame, 12, 1);
	for(int i = 0; i < frame.length; i++) {
	    if(frame[i] != 1) {
		throw new Error("Wavelet error: inverse");
	    }
	}
    }

    private static void fill(short arr[], int v) {
	short s = (short)v;
	for(int i = 0; i < arr.length; i++) {
	    arr[i] = s;
	}
    }
    
    private static String printable(short arr[], int w) {
	StringBuilder sb = new StringBuilder();
	for(int i = 0; i < arr.length; i++) {
	    sb.append(String.format("%c%d", (((i % w) == 0) ? '\n' : ' '),
				    arr[i]));
	}
	return sb.toString();
    }

}