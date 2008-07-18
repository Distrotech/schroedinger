FILES= Unpack.class Wavelet.class Decoder.class \
	Picture.class VideoFormat.class Buffer.class \
	Arithmetic.class ColourSpace.class \
	MotionVector.class

CLASSPATH= ../../../


all: ${FILES}
	@echo "Build!"

%.class: %.java
	javac $< -classpath ${CLASSPATH}

clean:
	@rm *.class