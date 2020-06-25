FRAMEWORK_DIR =./framework
MODEL_DIR =./model
CC=g++
CFLAGS=-I$(FRAMEWORK_DIR) -I$(MODEL_DIR) -std=c++17 -Xpreprocessor -fopenmp -ffast-math -g -lm -m64 -flto -march=native -Ofast

ODIR = ./out/

run: clear | $(ODIR)sim
	cd $(ODIR) && ./sim

$(ODIR)sim: | $(ODIR)
	$(CC) $(CFLAGS) -o $(ODIR)sim ./main.cpp

$(ODIR):
	mkdir $(ODIR)

clear:
	rm -r $(ODIR)
