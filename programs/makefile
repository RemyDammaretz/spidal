test: bin/test/spidal bin/test/remote bin/test/spidalDouble bin/test/remoteDouble
app: spidal remote
spidal: bin/app/spidal bin/app/spidalUi bin/app/spidalBack
remote: bin/app/remote
noRPI: bin/app/noRPI bin/app/noRPIUi bin/app/noRPIBack

################################################
## ------ Tests
################################################

bin/test/spidal: test/spidal.c src/libs/gpioLib.c src/libs/socketLib.c src/libs/appStatus.c src/libs/signalsLib.c
	gcc -Wall -pthread -lpigpio -lrt test/spidal.c src/libs/gpioLib.c src/libs/socketLib.c  src/libs/appStatus.c src/libs/signalsLib.c -o bin/test/spidal
	
bin/test/remote: test/remote.c src/libs/gpioLib.c src/libs/socketLib.c
	gcc -Wall -pthread -lpigpio -lrt test/remote.c src/libs/gpioLib.c src/libs/socketLib.c -o bin/test/remote
	

bin/test/spidalDouble: test/spidalDouble.c src/libs/gpioLib.c src/libs/socketLib.c src/libs/appStatus.c src/libs/signalsLib.c
	gcc -Wall -pthread -lpigpio -lrt test/spidalDouble.c src/libs/gpioLib.c src/libs/socketLib.c  src/libs/appStatus.c src/libs/signalsLib.c -o bin/test/spidalDouble
	
bin/test/remoteDouble: test/remoteDouble.c src/libs/gpioLib.c src/libs/socketLib.c
	gcc -Wall -pthread -lpigpio -lrt test/remoteDouble.c src/libs/gpioLib.c src/libs/socketLib.c -o bin/test/remoteDouble
	


################################################	
## ------ App
################################################

## Spidal

bin/app/spidal: src/app/spidal.c
	gcc -Wall -pthread src/app/spidal.c -o bin/app/spidal

bin/app/spidalUi: src/app/spidalUi.c src/libs/appStatus.c src/libs/signalsLib.c
	gcc -Wall -pthread src/app/spidalUi.c src/libs/appStatus.c src/libs/signalsLib.c -o bin/app/spidalUi -lm `pkg-config --cflags --libs gtk+-3.0` -rdynamic

# Needs to have root privilegies for suid bit
bin/app/spidalBack: src/app/spidalBack.c src/libs/gpioLib.c src/libs/socketLib.c src/libs/signalsLib.c
ifneq ($(shell id -u), 0)
	@echo "\033[0;31mYou are not root, run this target as root please\033[0m"
else
	gcc -Wall -pthread -lpigpio -lrt src/app/spidalBack.c src/libs/gpioLib.c src/libs/socketLib.c src/libs/signalsLib.c -o bin/app/spidalBack
	chmod u+s bin/app/spidalBack
endif

## No RPI

bin/app/noRPI: src/app/spidal.c
	gcc -Wall -pthread src/app/spidal.c -DNO_RPI -o bin/app/noRPI

bin/app/noRPIUi: src/app/spidalUi.c src/libs/appStatus.c src/libs/signalsLib.c
	gcc -Wall -pthread src/app/spidalUi.c src/libs/appStatus.c src/libs/signalsLib.c -DNO_RPI -o bin/app/noRPIUi -lm `pkg-config --cflags --libs gtk+-3.0` -rdynamic

bin/app/noRPIBack: src/app/noRPIBack.c
	gcc -Wall src/app/noRPIBack.c -DNO_RPI -o bin/app/noRPIBack

## Remote

# Needs to have root privilegies for suid bit
bin/app/remote: src/app/remote.c src/libs/gpioLib.c src/libs/socketLib.c src/libs/signalsLib.c
ifneq ($(shell id -u), 0)
	@echo "\033[0;31mYou are not root, run this target as root please\033[0m"
else
	gcc -Wall -pthread -lpigpio -lrt src/app/remote.c src/libs/gpioLib.c src/libs/socketLib.c -o bin/app/remote
	chmod u+s bin/app/remote
endif
