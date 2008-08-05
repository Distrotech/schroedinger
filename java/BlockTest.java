import org.diracvideo.Schroedinger.Block;
import java.awt.*;
import java.awt.image.*;

public class BlockTest {
    private static Frame win;
    private static Canvas can;
    private static Graphics gr;

    public static void main(String arg[]) {
	initWindow();
	testBlock();
	win.setVisible(true);
	win.dispose();
    }
    
    private static void initWindow() {
	win = new Frame("BlockTest");
	can = new Canvas();
	gr = can.getGraphics();
	win.add(can);
	win.setVisible(true);
    }

    private static void testBlock() {
	Block bl = new Block(new Dimension(200,200));
	bl.checkers(3);
	Block ch = bl.sub(new Point(55, 55), new Dimension(100,100));
	Block ot = new Block(new Dimension(100,100));
	ch.copyTo(ot);
	if(!ch.equals(ot)) {
	    System.err.println("Blocks not equal");
	} else {
	    System.err.println("Blocks are equal");
	}
	draw(ot);
    }

    private static void draw(Block bl) {
	BufferedImage img = new BufferedImage(bl.s.width, bl.s.height, 
					      BufferedImage.TYPE_INT_RGB);
	can.setSize(bl.s);
	win.pack();
	int pixels[] = new int[bl.s.width * bl.s.height];
	for(int i = 0; i < bl.s.height; i++) {
	    for(int j = 0; j < bl.s.width; j++) {
		pixels[i*bl.s.width + j] = bl.pixel(i,j)*0x010101;
	    }
	}
	img.setRGB(0,0, bl.s.width, bl.s.height, pixels, 0, bl.s.width);
	can.getGraphics().drawImage(img, 0, 0, null);
	try {
	    Thread.sleep(2000);
	} catch(InterruptedException x) {}	
    }
}