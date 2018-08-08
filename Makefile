jankenpon: jankenpon/jankenpon.cpp
	eosiocpp -o jankenpon/jankenpon.wast jankenpon/jankenpon.cpp
	eosiocpp -g jankenpon/jankenpon.abi jankenpon/jankenpon.cpp

clean:
	rm *.wasm *.wast
