import org.diracvideo.Schroedinger.*;

public class Wavelet2dTest {
    private static Wavelet wav;
    public static void main(String a[]) {
	wav = new Wavelet();
	synthesizeTest();
	interLeaveTest();
	twoDimensionTest();
    }

    private static void synthesizeTest() {
	short d[] = new short[16];
	d[8] = 9;
	d[12] = 5;
	d[14] = 2;
	d[15] = 1;
	wav.synthesize(d,4,0,d.length);
	wav.synthesize(d,2,0,d.length);
	wav.synthesize(d,1,0,d.length);
	System.err.println(printable(d,16));
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
	short frame[] = wav.interleave(ll,lh,hl,hh,6);
	//	System.err.println(printable(frame,12));
    }

    private static void twoDimensionTest() {
	short ll[] = new short[36];
	short other[] = new short[36];
	fill(ll,1);
	short frame[] = wav.interleave(ll, other, other, other, 6);
	wav.inverse(frame, 12, 1);
	System.err.println(printable(frame,12));
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