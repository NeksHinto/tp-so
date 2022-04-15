all:
	gcc -Wall -o view view.h
	gcc -Wall -o application application.h
	gcc -Wall -o worker worker.h
	gcc -Wall -o worker worker.c
	gcc -Wall -o view view.c -lrt -pthread
	gcc -Wall -o application application.c -lrt -pthread
	
clean:
	rm application worker view
	rm ./output_application
	rm /dev/shm/shm_obj

.PHONY: all clean

# a phony target is simply a target that is always out-of-date, 
# so whenever you ask make <phony_target>, it will run, independent 
# from the state of the file system.
# ignora la existencia de el/los archivo/s que estan despues
# cosa de “actualizar” el archivo	