import org.diracvideo.Schroedinger.*;
import java.io.*;


public final class DecoderTest {
    public static void main(String a[]) {
	Decoder dec = new Decoder();
	int ev = 0;
	FileInputStream in = null;
	try {
	    in = tryOpen(a);
	    byte[] packet;
	    Picture output;
	    while(in.available() > 0) {
		packet = readPacket(in);
		dec.push(packet);
		dec.run();
		output = dec.pull();
	    }
	    in.close();
	} catch(IOException e) {
	    e.printStackTrace();
	    ev = 1;
	}  catch(Exception e) {
	    e.printStackTrace();
	    ev = 1;
	} finally {
	    System.exit(ev);
	}
    }

    private static byte[] readPacket(FileInputStream in) throws IOException {
	byte[] header = new byte[13];
	in.read(header);
	Unpack u = new Unpack(header);
	if(u.decodeLit32() != 0x42424344) {
	    throw new IOException("Cannot parse dirac stream");
	} 
	if(u.bits(8) == 0x10) {
	    exitGracefully(in);
	}
	int size = u.decodeLit32();
	byte[] packet = new byte[size];
	System.arraycopy(header, 0, packet, 0, 13);
	in.read(packet, 13, size - 13);
	return packet;
    }

    private static void dumpBytes(byte[] d) {
	for(int i = 0; i < d.length; i++) {
	    System.err.format("%02X ", d[i]);
	}
	System.err.println("");
    }

    private static void exitGracefully(FileInputStream in) {
	System.err.println("End of sequence reached, exiting");
	try {
	    in.close();
	} catch(IOException e) {
	    e.printStackTrace();
	    System.exit(1);
	}
	System.exit(0);
    }

    private static class DiracAcceptor implements FileFilter {
	public boolean accept(File f) {
	    String fn = f.getName();
	    if(fn.length() == fn.indexOf(".drc") + 4 &&
	       f.isFile() && f.canRead()) {
		return true;
	    }
	    return false;
	}
    }
    
    private static FileInputStream tryOpen(String a[]) throws IOException {
	for(int i = 0; i < a.length; i++) {
	    File f = new File(a[i]);
	    if (f.canRead()) {
		return new FileInputStream(f);
	    }
	}
	File[] files = new File(".").listFiles(new DiracAcceptor());
	for(int i = 0; i < files.length; i++) {
	    try {
		return new FileInputStream(files[i]);
	    } catch(IOException e) {
		e.printStackTrace();
	    }
	}
	System.err.println("No dirac file was found");
	System.exit(0);
	return null;
    }
}