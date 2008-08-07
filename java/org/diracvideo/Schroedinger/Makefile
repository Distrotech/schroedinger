CLASSPATH= ../../../
WAVELETS=DeslauriesDebuc9_7 LeGall5_3 DeslauriesDebuc13_7 \
	HaarSingleShift HaarNoShift Fidelity Daubechies9_7


all: Decoder.class
	@echo "Build!"

Decoder.java: Picture.class Queue.class Buffer.class

Picture.java: Parameters.class SubBand.class Slice.class \
	 Wavelet.class VideoFormat.class Arithmetic.class Motion.class \
	 Block.class ColourSpace.class

Slice.java: Unpack.class

SubBand.java: Arithmetic.class Unpack.class

VideoFormat.java: Unpack.class

Arithmetic.java: Context.class

Motion.java: Parameters.class Vector.class Block.class Global.class

Parameters.java: Global.class Wavelet.class

Global.java: Vector.class

%.class: %.java
	javac $< -classpath ${CLASSPATH}


clean:
	@rm -f *.class