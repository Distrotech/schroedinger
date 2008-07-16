FILES = UnpackTest.class Wavelet2dTest.class DecoderTest.class

test: lib ${FILES} 
	@java UnpackTest
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

