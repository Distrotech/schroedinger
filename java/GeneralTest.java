import java.awt.Dimension;
import java.awt.Point;

public class GeneralTest {
    public static void main(String a[]) {
	testLevel();
	testDivision();

    }

    private static void testLevel() {
	for(int n = 1; n < 8; n++) {
	    System.err.format("Testing with TransformDepth = %d\n", n);
	    for(int i = 0; i < 3*n+1; i++) {
		System.err.format("Number: %d\tLevel: %d\tOrient: %d\n", 
				  i, (i-1)/3, (i-1) % 3 + 1);
	    } 
	}
    }

    private static void testDivision() {
	for(int i = 0; i < 1000; i++) {
	    for(int j = 1; j < 1000; j++) {
		int d = i/j;
		if(d*j > i) {
		    System.err.format("Divison error");
		} else if(d*j != i) {
		    System.err.println("Inexact division");
		}
	    }
	}
    }

    private static void testBlockDimensions() {
	Dimension frame = new Dimension(320,240);
	for(int numY = 1; numY < 10; numY ++) {
	    for(int numX = 1; numX < 10; numX++) {
		System.err.format("numX: %d\tnumY: %d\n", numX, numY);
		Dimension block = new Dimension(frame.width / numX,
						frame.height / numY);
		for(int i = 0; i < numY; i++)
		    for(int j = 0; j < numX; j++) {
			int testStart = (block.width*j) +
			    (frame.width*block.height*i);
			int testEnd = testStart + block.width +
			    (frame.width*(block.height-1));
			int specX = (frame.width * j)/numX;
			int specY = (frame.height *i)/numY;
			int specStart = (frame.width*specY) + specX;
			int specEndX = (frame.width * (j+1))/numX;
			int specEndY = (frame.height *(i+1))/numY;
			int specEnd = (frame.width*(specEndY - 1)) + specEndX;
			if(specEnd != testEnd ||
			   specStart != testStart) {
			    System.err.format("Spec:\t\tTest\n%d\t\t%d\n",
					      specEnd, testEnd);
			    System.err.format("%d\t\t%d\n", specStart,
					      testStart);
			}
		    }
	    }
	}
    }
}