FILES = UnpackTest.class TestWavelet2d.class

test: lib ${FILES}
	@java UnpackTest
	@java TestWavelet2d


%.class: %.java
	javac $<

lib: ./Schroedinger
	make -C ./Schroedinger


clean:
	@find -name "*.class" -delete
	@find -name "*~" -delete
	@echo "Done"