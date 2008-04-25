import Schroedinger.*;

public class UnpackTest {
    public static void main (String a[]) {
	String s = "Hello, World!";
	Unpack u = new Unpack(s.getBytes());
	while(u.bitsLeft() > 8) {
	    System.out.println((char)u.bits(8));
	}
    }
}