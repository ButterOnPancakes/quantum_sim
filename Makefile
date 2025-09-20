bin/%.o: %.c
	mkdir -p $(dir $@)
	gcc -c $< -o $@ -lm

bin/computer/computer_test: bin/computer/circuit.o bin/computer/computer_test.o bin/utils/dynarrays/dynarray.o
	mkdir -p bin/computer
	gcc -o bin/computer/computer_test bin/computer/computer_test.o bin/computer/circuit.o bin/utils/dynarrays/dynarray.o -lm
test_computer: bin/computer/computer_test
	./bin/computer/computer_test

bin/matrices/matrices_test: bin/complex/complexes.o bin/matrices/matrices.o bin/matrices/matrices_test.o bin/utils/utils.o
	mkdir -p bin/matrices
	gcc -o bin/matrices/matrices_test bin/complex/complexes.o bin/matrices/matrices.o bin/matrices/matrices_test.o bin/utils/utils.o -lm
test_matrices: bin/matrices/matrices_test
	./bin/matrices/matrices_test

bin/complex/complex_test: bin/complex/complexes.o bin/complex/complex_test.o bin/utils/utils.o
	mkdir -p bin/complex
	gcc -o bin/complex/complex_test bin/complex/complexes.o bin/complex/complex_test.o bin/utils/utils.o -lm
test_complex: bin/complex/complex_test
	./bin/complex/complex_test

clean:
	rm -rf bin/*