import org.diracvideo.Schroedinger.*;

public class Wavelet2dTest {
    public static void main(String a[]) {
	synthesizeTest();
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


}