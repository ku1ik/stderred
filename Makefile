stderred.so: hack_write.c
	gcc hack_write.c -ldl -shared -o stderred.so
