import org.diracvideo.Schroedinger.*;

public class TestWavelet2d {
    public static void main(String a[]) {
	System.out.println("Hello, Wavelets!");
	short d[] = new short[15];
	short t[] = new short[20];
	WaveletTransform.InverseLeGall5_3(d,0,0,0,t);
    }
}