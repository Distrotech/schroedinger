import Schroedinger.*;


public class test {
    public static void main (String [] a) {
	byte d[];
	String s = "Hello, World!";
	d = s.getBytes();
	Unpack u = new Unpack(d);
	u.bits(5);
	u.align();
	while(u.bitsLeft() >= 8) {
	    System.out.println((char)u.bits(8));
	}
    }
}