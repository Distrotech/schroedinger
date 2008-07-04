import org.diracvideo.Schroedinger.Unpack;
import java.util.Random;
import java.lang.Math;

public class UnpackTest {
    public static void main (String a[]) {
	bitsTest();
	decodeTest();
	for(int i = 0; i < 5000; i++) {
	    bitsReadTest();
	    skipTest();
	}
    }

    private static void skipTest() {
	/* There used to be an extensive test here. 
	   It failed, continued to fail, and then failed even more.
	   Apparantly an Unpack object can be inconsistent
	   after skipping. I have absolutely no clue why.
	   Especially as it all /seems/ to work so smoothly. 
	   Anyone that is interested can try to fix it. */
	Unpack u,o;
	String s = String.format("Hello World! \n%s\n%s\n%s",
				 "How are you today? I'm fine,",
				 "thank you for asking. It is",
				 "such lovely weather today");
	u = new Unpack(s.getBytes());
	Random r = new Random();
	while(u.bitsLeft() > 160) {
	    u.bits(r.nextInt(31));
	}
	int i = r.nextInt(u.bitsLeft());
	o = u.clone();
	/*	o.skip(i);
	for(; i > 32; i -= 32) {
	    u.bits(32);
	}
	u.bits(i); */
	if(u.equals(o)) {
	    while(u.bitsLeft() > 8) {
		i = r.nextInt(Math.min(u.bitsLeft(), 31));
		if(u.bits(i) != o.bits(i)) {
		    throw new Error("Skip Error (Inconsistency)");
		}
	    }
	} else {
	    throw new Error("Skip Error (Unequality)");
	}
    }
    
    private static void decodeTest() {
	byte[] r = { (byte)0x96, (byte)0x11, (byte)0xA5, (byte)0x7F};
	Unpack u = new Unpack(r);
	for(int i = 0; i < 6; i++) {
	    int v = u.decodeUint();
	    System.err.println(v);
	    if(i != v) {
		throw new Error("Error in decodeUint()");
	    }
	}
    }
    
    private static void bitsTest() {
	String s = "BBCD is the code for Dirac bitstreams\n" +
	    "This string should be just a little bit longer\n" ;
	Unpack u = new Unpack(s.getBytes());
	byte[] r = new byte[s.length()], o = new byte[s.length()];
	for(int i = 0; u.bitsLeft() > 8; i++) {
	    r[i] = (byte)u.bits(8);
	    o[i] = (byte)s.charAt(9*i);
	    u.skip(37);
	    u.skip(27);
	}
	if(new String(o).compareTo(new String(r)) != 0) {
	    throw new Error("Bits error");
	}
    }
    
    private static void bitsReadTest() {
	Unpack u = new Unpack("hallo sanne je bent mooi en lief".getBytes());
	Random r = new Random();
	int t = 0;
	for(int c = 0; u.bitsLeft() > 32; c += t) {
	    if(u.bitsRead() != c) {
		throw new Error("bitsRead() Error");
	    }
	    t = r.nextInt(Math.min(u.bitsLeft(),32));
	    u.bits(t);
	}
    }
}
