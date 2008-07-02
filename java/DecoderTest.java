import org.diracvideo.Schroedinger.*;
import java.io.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;

class CloseListener extends WindowAdapter {
    public void windowClosing(WindowEvent e) {
 	System.exit(0);
    }
}

class PictureDrawer extends Canvas {
    private Decoder dec;

    public PictureDrawer(Decoder d) {
	dec = d;
    }
    
    public void paint(Graphics gr) {
	if(dec.hasPicture()) {
	    Picture pic = dec.pull();
	    Image img = pic.getImage();
	    gr.drawImage(img,0,0,null);
	} 
    }
}


class DiracAcceptor implements FileFilter {
    public boolean accept(File f) {
	String fn = f.getName();
	if(fn.length() == fn.indexOf(".drc") + 4 &&
	   f.isFile() && f.canRead()) {
	    return true;
	}
	return false;
    }
}

public final class DecoderTest {
    public static void main(String a[]) {
	Decoder dec = new Decoder();
	int ev = 0;
	FileInputStream in = null;
	Frame win;
	try {
	    in = tryOpen(a);
	    byte[] packet;
	    Picture output;
	    packet = readPacket(in);
	    dec.push(packet);
	    win = createWindow(dec);
	    while(in.available() > 0) {
		packet = readPacket(in);
		dec.push(packet);
		win.repaint();
		if(dec.status == Decoder.Status.DONE) {
		    break;
		}
	    }
	    in.close();
	} catch(IOException e) {
	    e.printStackTrace();
	    ev = 1;
	}  catch(Exception e) {
	    e.printStackTrace();
	    ev = 1;
	}  finally { 
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
	    return header;
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

    private static Frame createWindow(Decoder dec) {
	VideoFormat f = dec.getVideoFormat();
	Frame fr = new Frame("DecoderTest");
	Canvas cn = new PictureDrawer(dec);
	WindowListener wl = new CloseListener();
	fr.add(cn);
	fr.addWindowListener(wl);
	fr.setSize(f.width, f.height);
	fr.setVisible(true);
	return fr;
    }


}