suit:suit.cc
	g++ -o $@ $^ -std=c++11 -lgtest

test:test.cc
	g++ -o $@ $^ -std=c++11 -lgtest

assert:assert.cc	
	g++ -o $@ $^ -std=c++11 -lgtest
.PHONY:clean
clean:
	rm -f assert test