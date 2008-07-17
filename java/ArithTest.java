import org.diracvideo.Schroedinger.*;
import java.io.*;

/** A test for the arithmetic decoder.
 *
 * I'll create a test program for the 
 * schroedinger arithmetic encoder which will 
 * create a pattern that ArithTest should decode **/

class ArithTest {

    public static void main(String a[]) {
	File f = new File("out.arith");
	ArithmeticDecoder d;
	try {
	    Buffer b = readAll(f);
	    d = new ArithmeticDecoder(b);
	} catch(IOException e) {
	    System.err.println("Could not open test file");
	    System.exit(1);
	    return;
	}
	testArithmeticDecoder(d);
    }

    private static Buffer readAll(File f) throws IOException {
	InputStream i = new FileInputStream(f);
	int a = i.available();
	byte b[] = new byte[a];
	i.read(b);
	i.close();
	return new Buffer(b);
    }
    
    private static void testArithmeticDecoder(ArithmeticDecoder a) {
	for(int i = 0; i < 100; i++) {
	    System.err.format("%d\n", a.decodeSint(0,0,0));
	}
    }
}

