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

class PictureDrawer extends Canvas implements Runnable {
    private Decoder dec;
    private int wait;
    
    public PictureDrawer(Decoder d) {
	dec = d;
	VideoFormat fr = dec.getVideoFormat();
	wait = (1000 * fr.frame_rate_denominator) / fr.frame_rate_numerator;
    }


    public void run() {
	Graphics gr = getGraphics();
	while(dec.status != Decoder.Status.DONE) {
	    paint(gr);
	    try {
		Thread.sleep(wait);
	    } catch(InterruptedException e) {
		e.printStackTrace();
	    }
	}
    }
    
    public void paint(Graphics gr) {
	if(dec.hasPicture()) {
	    Picture pic = dec.pull();
	    //	    System.err.println(pic);
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
    private static Thread other_thread;
    public static void main(String a[]) {
	Decoder dec = new Decoder();
	int ev = 0, tm;
	FileInputStream in = null;
	Frame win;
	try {
	    in = tryOpen(a);
	    byte[] packet;
	    while(dec.getVideoFormat() == null) {
		packet = readPacket(in);
		dec.push(packet);
	    }
	    win = createWindow(dec);
	    while(in.available() > 0) {
		packet = readPacket(in);
		dec.push(packet);
		if(dec.status == Decoder.Status.DONE) {
		    break;
		}
	    }
	    dec.status = Decoder.Status.DONE;
	    in.close();
	    other_thread.join();
	    win.setVisible(false);
	    win.dispose();
	} catch(IOException e) {
	    e.printStackTrace();
	    ev = 1;
	} catch(InterruptedException e) {
	    System.err.println(e);
	} catch(Exception e) {
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
	    return header;
	}
	int size = u.decodeLit32();
	byte[] packet = new byte[size];
	System.arraycopy(header, 0, packet, 0, 13);
	in.read(packet, 13, size - 13);
	return packet;
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
	PictureDrawer cn = new PictureDrawer(dec);
	WindowListener wl = new CloseListener();
	cn.setSize(f.width, f.height);
	fr.add(cn);
	fr.pack();
	fr.addWindowListener(wl);
	fr.setVisible(true);
	other_thread = new Thread(cn);
	other_thread.start();
	return fr;
    }


}