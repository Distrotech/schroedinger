FILES = UnpackTest.class Wavelet2dTest.class DecoderTest.class

test: lib ${FILES} 
#	@java UnpackTest
	@java Wavelet2dTest	
	@java DecoderTest Jabberwocky.drc

lib:
	make -C ./org/diracvideo/Schroedinger


%.class: %.java
	javac $<


clean:
	@find -name "*.class" -delete
	@find -name "*~" -delete
	@echo "Done"