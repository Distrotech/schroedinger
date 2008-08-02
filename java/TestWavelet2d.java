import org.diracvideo.Schroedinger.*;

public class TestWavelet2d {
    public static void main(String a[]) {
	System.out.println("Hello, Wavelets!");
	short d[] = new short[16*16];
	WaveletTransform.inverse(d,16);
    }
}