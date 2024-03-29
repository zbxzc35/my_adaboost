OBJ = cascade_classifier.o strong_classifier.o weak_classifier.o feature.o tool.o
DEP = -I/usr/local/include/opencv -lopencv_core -lopencv_imgproc -lopencv_highgui
DIR = bin
DST = ${DIR}/train ${DIR}/detect ${DIR}/samples ${DIR}/main

all: outdir ${DST}

outdir: 
	mkdir -p bin

${DIR}/train:main.cpp ${OBJ}
	g++ -DTRAIN_MODEL -O3 $^ -o $@ ${DEP}

${DIR}/detect:main.cpp ${OBJ}
	g++ -DDETECT -O3 $^ -o $@ ${DEP}

${DIR}/samples:main.cpp ${OBJ}
	g++ -DGENERATE_SAMPLE -O3 $^ -o $@ ${DEP}

${DIR}/main:main.cpp ${OBJ}
	g++  -O3 $^ -o $@ ${DEP}

${OBJ}:%.o:%.cpp
	g++ -c -O3 $^

clean:
	rm ${OBJ} ${DST} -f
