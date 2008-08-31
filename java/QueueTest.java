import org.diracvideo.Schroedinger.*;
import java.awt.Image;


public class QueueTest {
    public static void main(String args[]) {
	explicitRemoveTest();
	popTest();
	implicitRemoveTest();
	System.out.println("Queue implementation OK");
    }

    private static void explicitRemoveTest() {
	Queue queue = new Queue(5);
	for(int i = 0; i < 2000; i++) {
	    Picture pic = new Picture(i);
	    queue.add(pic);
	    if(i > 3) {
		if(queue.get(i - 4) == null) {
		    throw new Error("Queue implementaion error");
		}
		queue.remove(i - 4);
		if(queue.get(i - 4) != null) {
		    throw new Error("Queue implementation error");
		}
	    }
	}
    }

    private static void popTest() {
	Queue queue = new Queue(5);
	for(int i = 0; i < 2000; i++) {
	    Picture pic = new Picture(i);
	    queue.add(pic);
	    if(queue.full()) {
		for(int j = i - 4; j <= i; j++) 
		    if(j != queue.pop().num) 
			throw new Error("Queue implementation error");
	    }
	}
    }

    private static void implicitRemoveTest() {
	Queue queue = new Queue(5);
	for(int i = 0; i < 2000; i++) {
	    Picture pic = new Picture(i);
	    queue.add(pic);
	    if(i > 4) {
		if(queue.get(i - 4) == null) 
		    throw new Error("Queue implementation error");
		if(queue.get(i - 5) != null)
		    throw new Error("Queue implementation error");
	    }
	}
    }
}