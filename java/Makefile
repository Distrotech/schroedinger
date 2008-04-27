FILES = UnpackTest.class TestWavelet2d.class 

test: ${FILES}
	@java UnpackTest
	@java TestWavelet2d


%.class: %.java
	javac $<


clean:
	@find -name "*.class" -delete
	@find -name "*~" -delete
	@echo "Done"