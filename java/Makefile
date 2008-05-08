FILES = UnpackTest.class TestWavelet2d.class 

test: lib ${FILES} 
	@java UnpackTest
	@java TestWavelet2d

lib:
	make -C ./org/diracvideo/Schroedinger


%.class: %.java
	javac $<


clean:
	@find -name "*.class" -delete
	@find -name "*~" -delete
	@echo "Done"