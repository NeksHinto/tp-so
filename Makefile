CC = gcc
GCCFLAG = -g -Wall -std=c99
GCCLIBS = -lrt -lpthread
EXT_FILES = utils.c
APP = application view worker
OUTPUT = output_application

all: $(APP)
$(APP): %: %.c
	@$(CC) $(GCCFLAGS) $(EXT_FILES) -o $@ $< $(GCCLIBS)

.PHONY: clean
clean:
	@rm -rf $(APP) $(OUTPUT)

# # a phony target is simply a target that is always out-of-date, 
# # so whenever you ask make <phony_target>, it will run, independent 
# # from the state of the file system.
# # ignora la existencia de el/los archivo/s que estan despues
# # cosa de “actualizar” el archivo	

# 	@$(CC) $(GCCFLAGS) $(EXT_FILES) -o $@ $< $(GCCLIBS)
