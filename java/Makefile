FILES = UnpackTest.class Wavelet2dTest.class DecoderTest.class\

test: lib ${FILES} 
	@java DecoderTest 

lib:
	make -C ./org/diracvideo/Schroedinger


%.class: %.java
	javac $<


clean: cleandoc
	@find -name "*.class" -delete
	@find -name "*~" -delete
	@rm -f *.drc *.sundae
	@echo "Done"

doc: cleandoc
	mkdir docs
	javadoc -d docs org.diracvideo.Schroedinger

cleandoc:
	@rm -rf docs

