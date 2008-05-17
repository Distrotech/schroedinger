FILES= Unpack.class WaveletTransform.class Decoder.class \
	Picture.class VideoFormat.class
CLASSPATH= ../../../


all: ${FILES}
	@echo "Build!"

%.class: %.java
	javac $< -classpath ${CLASSPATH}