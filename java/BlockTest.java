import org.diracvideo.Schroedinger.Block;
import java.awt.*;
import java.awt.image.*;

class PictureCanvas extends Canvas {
    private Image img;

    public PictureCanvas() {
	img = new BufferedImage(100, 100, BufferedImage.TYPE_INT_RGB);
	setSize(new Dimension(100,100));
    }

    public void setImage(Image i) {
	img = i;
	paint(getGraphics());
    }
	    
    public void paint(Graphics gr) {
	gr.drawImage(img, 0, 0, null);
    }
}

public class BlockTest {
    private static Frame win;
    private static PictureCanvas can;
    private static Graphics gr;

    public static void main(String arg[]) {
	initWindow();
	testBlock();
	destroyWindow();
    }
    
    private static void initWindow() {
	win = new Frame("BlockTest");
	can = new PictureCanvas();
	win.add(can);
	win.pack();
	win.setVisible(true);
    }

    private static void destroyWindow() {
	win.setVisible(false);
	win.dispose();
	System.exit(0);
    }

    private static void testBlock() {
	Block bl = new Block(new Dimension(200,200));
	bl.checkers(3);
	Block ch = bl.sub(new Point(110, 110), new Dimension(100,100));
	Block ot = new Block(new Dimension(100,100));	
	ch.copyTo(ot);
	Block up = ot.upSample();
	draw(ot);
	safeSleep(1000);
	draw(up);
	safeSleep(1000);
    }

    private static void safeSleep(int i) {
	try {
	    Thread.sleep(i);
	} catch(InterruptedException x) {}
    }

    private static void draw(Block bl) {
	BufferedImage img = new BufferedImage(bl.s.width, bl.s.height, 
					      BufferedImage.TYPE_INT_RGB);
	int pixels[] = new int[bl.s.width * bl.s.height];
	for(int i = 0; i < bl.s.height; i++) {
	    for(int j = 0; j < bl.s.width; j++) {
		pixels[i*bl.s.width + j] = bl.pixel(i,j)*0x010101;
	    }
	}
	img.setRGB(0,0, bl.s.width, bl.s.height, pixels, 0, bl.s.width);
	can.setImage(img);
    }
}