jankenpon: jankenpon.cpp
	eosiocpp -o jankenpon.wast jankenpon.cpp
	eosiocpp -g jankenpon.abi jankenpon.cpp

clean:
	rm *.wasm *.wast
