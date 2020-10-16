main: main.cpp concert.o ipc_util.o
	g++ -o $@ $^

ipc_util: ipc_util.cpp ipc_util.h transaction.h ipc_exception.h
	g++ -c $^

concert.o: concert.cpp ipc_util.o 
	g++ -c $^

.PHONY : clean
clean :
	-rm main *.o *.h.gch