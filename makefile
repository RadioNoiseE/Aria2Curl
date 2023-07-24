.PHONY : complie-wasm
complie-wasm : aria2curl.c
	clang aria2curl.c -o aria2curl.wasm
