FILES = UnpackTest.class Wavelet2dTest.class DecoderTest.class\
	ArithTest.class GeneralTest.class

test: lib ${FILES} 
	@java UnpackTest
	@java ArithTest
	@java Wavelet2dTest	
	@java DecoderTest 


lib:
	make -C ./org/diracvideo/Schroedinger


%.class: %.java
	javac $<


clean: cleandoc
	@find -name "*.class" -delete
	@find -name "*~" -delete
	@rm -f *.drc
	@echo "Done"

doc:
	javadoc org.diracvideo.Schroedinger

cleandoc:
	@find -name "*.html" -delete
	@find -name "*.css" -delete
	@rm -rf resources/
	@rm -f package-list

