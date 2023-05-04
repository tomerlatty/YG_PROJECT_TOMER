OBJECTS= ./build/cmplr.o ./build/cprcss.o ./build/lxr_prt.o ./build/systm_tkm.o ./build/prsr_prt.o ./build/systm_scp.o ./build/systm_sym.o ./build/systm_nd.o ./build/assistant.o ./build/structures/buffer.o ./build/structures/vector.o
INCLUDES= -I./

all: ${OBJECTS}
	gcc main.c ${INCLUDES} ${OBJECTS} -g -o ./main

./build/cmplr.o: ./cmplr.c
	gcc cmplr.c ${INCLUDES} -o ./build/cmplr.o -g -c

./build/cprcss.o: ./cprcss.c
	gcc cprcss.c ${INCLUDES} -o ./build/cprcss.o -g -c

./build/lxr_prt.o: ./lxr_prt.c
	gcc lxr_prt.c ${INCLUDES} -o ./build/lxr_prt.o -g -c

./build/systm_tkm.o: ./systm_tkm.c
	gcc systm_tkm.c ${INCLUDES} -o ./build/systm_tkm.o -g -c

./build/prsr_prt.o: ./prsr_prt.c
	gcc prsr_prt.c ${INCLUDES} -o ./build/prsr_prt.o -g -c

./build/systm_nd.o: ./systm_nd.c
	gcc systm_nd.c ${INCLUDES} -o ./build/systm_nd.o -g -c

./build/systm_scp.o: ./systm_scp.c
	gcc systm_scp.c ${INCLUDES} -o ./build/systm_scp.o -g -c

./build/systm_sym.o: ./systm_sym.c
	gcc systm_sym.c ${INCLUDES} -o ./build/systm_sym.o -g -c

./build/assistant.o: ./assistant.c
	gcc assistant.c ${INCLUDES} -o ./build/assistant.o -g -c

./build/structures/buffer.o: ./structures/buffer.c
	gcc ./structures/buffer.c ${INCLUDES} -o ./build/structures/buffer.o -g -c

./build/structures/vector.o: ./structures/vector.c
	gcc ./structures/vector.c ${INCLUDES} -o ./build/structures/vector.o -g -c

clean:
	rm ./main
	rm -rf ${OBJECTS}
