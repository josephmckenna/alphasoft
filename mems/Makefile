all: mems libiobb.a

libiobb.a:
	make -C iobb libiobb.a

mems: mems.c libiobb.a
	gcc -Iiobb -o mems mems.c -Liobb -liobb

clean:
	rm -rf mems
	cd iobb/ && $(MAKE) clean
