import org.diracvideo.Schroedinger.*;
import java.io.*;


public final class DecoderTest {
    public static void main(String a[]) {
	Decoder dec = new Decoder();
	int i = 43923902;
	System.out.format("%X\n",i);
	i <<= 16;
	System.out.format("%X\n",i);
	FileInputStream in;
	try {
	    in = new FileInputStream(a[0]);
	    byte[] packet;
	    byte[] output;
	    while(in.available() > 0) {
		packet = readPacket(in);
		dec.push(packet);
		output = dec.pull();
	    }
	    in.close();
	} catch(IOException e) {
	    e.printStackTrace();
	    System.exit(1);
	}  catch(Exception e) {
	    e.printStackTrace();
	    System.exit(1);
	}
    }

    private static byte[] readPacket(FileInputStream in) throws IOException {
	byte[] header = new byte[13];
	in.read(header);
	Unpack u = new Unpack(header);
	if(u.decodeLit32() != 0x42424344) {
	    throw new IOException("Cannot parse dirac stream");
	} 
	System.out.println(u);
	u.bits(8);
	System.out.println(u);
	int size = u.decodeLit32();
	
	System.out.format("%X\n",size);
	byte[] packet = new byte[size];
	System.arraycopy(header, 0, packet, 0, 13);
	in.read(packet, 13, size - 13);
	return packet;
    }
}