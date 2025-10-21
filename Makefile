bin/%.o: %.c
	mkdir -p $(dir $@)
	gcc -c $< -o $@ -lm

bin/computer/computer_test: bin/computer/computer_test.o bin/computer/circuit.o  bin/utils/lists/list.o bin/utils/utils.o bin/matrices/matrices.o
	mkdir -p bin/computer
	gcc -fopenmp -g -fsanitize=address -Wall -o bin/computer/computer_test bin/computer/computer_test.o bin/computer/circuit.o bin/utils/lists/list.o bin/utils/utils.o bin/matrices/matrices.o -lm
test_computer: bin/computer/computer_test
	./bin/computer/computer_test

bin/computer/grover: bin/computer/grover.o bin/computer/circuit.o  bin/utils/lists/list.o bin/utils/utils.o bin/matrices/matrices.o
	mkdir -p bin/computer
	gcc -fopenmp -g -fsanitize=address -Wall -o bin/computer/grover bin/computer/grover.o bin/computer/circuit.o bin/utils/lists/list.o bin/utils/utils.o bin/matrices/matrices.o -lm
grover: bin/computer/grover
	./bin/computer/grover


clean:
	rm -rf bin/*