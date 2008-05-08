FILES= Unpack.class WaveletTransform.class Decoder.class \
	Picture.class Parameters.class
CLASSPATH= ../../../


all: ${FILES}
	@echo "Build!"

%.class: %.java
	javac $< -classpath ${CLASSPATH}