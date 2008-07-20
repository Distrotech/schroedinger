import org.diracvideo.Schroedinger.*;
import java.io.*;

/** A test for the arithmetic decoder.
 *
 * Decodes a arithmetic encoded file from arith_file */

class ArithAcceptor implements FileFilter {
    public boolean accept(File f) {
	String fn = f.getName(); 
	if(fn.length() == fn.lastIndexOf(".sundae") + 7 &&  
	   f.isFile() && f.canRead()) {
	    return true;
	}
	return false;
    }
}

class ArithTest {
    public static void main(String a[]) {
	try {
	    FileInputStream in = tryOpen(a);
	    byte d[] = readAll(in);
	    in.close();
	    Arithmetic ar = new Arithmetic(d);
	    testArithmetic(ar);
	} catch(IOException e) {
	    e.printStackTrace();
	}
    }

    private static FileInputStream tryOpen(String a[]) throws IOException {
	for(int i = 0; i < a.length; i++) {
	    File f = new File(a[i]);
	    if (f.canRead()) {
		return new FileInputStream(f);
	    }
	}
	File[] files = new File(".").listFiles(new ArithAcceptor());
	for(int i = 0; i < files.length; i++) {
	    try {
		return new FileInputStream(files[i]);
	    } catch(IOException e) {
		e.printStackTrace();
	    }
	}
	System.err.println("No arith file was found");
	System.exit(0);
	return null;
    }
    
    private static byte[] readAll(FileInputStream in) throws IOException {
	byte d[] = new byte[in.available()];
	in.read(d);
	return d;
    }

    private static void testArithmetic(Arithmetic a) {
	byte d[] = new byte[a.bytesLeft()];
	for(int i = 0; i < d.length; i++) {
	    byte c = 0;
	    for(int j = 0; j < 8; j++) {
		c = (byte)((c << 1) | a.decodeBit(0));
	    }
	    d[i] = c;
	}
	String s = new String(d);
	System.out.println(s);
    }
}

