import org.diracvideo.Schroedinger.*;

public class UnpackTest {
    public static void main (String a[]) {
	String s = "Hello, World!";
	Unpack u = new Unpack(s.getBytes());
	u.bits(5);
	u.align();
	while(u.bitsLeft() > 8) {
	    System.out.print((char)u.bits(8));
	}
	System.out.println("");
    }
}
