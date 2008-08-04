FILES= Unpack.class Wavelet.class Decoder.class \
	Queue.class Picture.class VideoFormat.class \
	Buffer.class Arithmetic.class ColourSpace.class \
	Motion.class Block.class Context.class

CLASSPATH= ../../../

all: ${FILES}
	@echo "Build!"

%.class: %.java
	javac $< -classpath ${CLASSPATH}

clean:
	@rm -f *.class