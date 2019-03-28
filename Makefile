
TARGET=app.exe
src=$(wildcard *.cpp)

$(TARGET): mongoose.o main.o
	g++ -o $@ $^ -lws2_32

mongoose.o: mongoose.c
	gcc -O3 -o $@ -c $<

main.o: $(src)
	g++ -O3 -o $@ -c main.cpp

.PHONY: run
run: $(TARGET)
	./$(TARGET)

.PHONY: clean
clean:
	del /f *.o
	del /f *.exe
	del /f *.out	