objects = complex/complex_test

bin/%.o: %.c
	mkdir -p $(dir $@)
	gcc -c $< -o $@ -lm

bin/complex/complex_test: bin/complex/complexes.o bin/complex/complex_test.o bin/utils/utils.o
	mkdir -p bin/complex
	gcc -o bin/complex/complex_test bin/complex/complexes.o bin/complex/complex_test.o bin/utils/utils.o -lm

test_complex: bin/complex/complex_test
	./bin/complex/complex_test

clean:
	rm -rf bin/*