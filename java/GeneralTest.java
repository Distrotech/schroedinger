public class GeneralTest {
    public static void main(String a[]) {
	testLevel();
	assertTest();
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

    private static void assertTest() {
	assert false : "foo";
    }
}