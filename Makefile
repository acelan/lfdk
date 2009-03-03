

all:
	mkdir bin
	$(MAKE) -C lfdd
	$(MAKE) -C lfdk
	cp -f lfdd/lfdd_drv.ko bin
	cp -f lfdk/lfdk bin

clean:
	$(MAKE) -C lfdd clean
	$(MAKE) -C lfdk clean
	rm -rf bin


