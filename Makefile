CXX = em++
CXXFLAGS = -std=c++20 -Wall -Werror \
	-O2 --closure 1 -s ENVIRONMENT=worker
	# -s FILESYSTEM=0 
export EMCC_DEBUG=1

web: obj/wasm.o obj/2048.o
	$(CXX) $(CXXFLAGS) --bind -o js/solve.js $^ \
		-s WASM=1 -s ALLOW_MEMORY_GROWTH=1

obj/wasm.o: cpp/wasm.cpp | obj
	$(CXX) $(CXXFLAGS) -c -o $@ $<
obj/2048.o: cpp/2048.cpp | obj
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj:
	mkdir -p $@

.PHONY: clean
clean:
	rm -r js/solve.js js/solve.wasm obj/
