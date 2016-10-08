ifeq ($(BUILD),debug)
	FLAGS = -Og -Wall -g -DDEBUG
else
	#FLAGS = -O3 -Wall -Werror -s -DNDEBUG
	FLAGS = -O3 -Wall -s -DNDEBUG
endif

GENERAL_HELPER = ~/programming_projects/c/general/bin

PKG_CONFIG_LIBS = glfw3 glew glib-2.0 assimp

bin/main: bin/obj_model bin/obj_parser bin/camera bin/shader
	gcc $(FLAGS) `pkg-config --cflags $(PKG_CONFIG_LIBS)` \
	 	-o bin/main bin/obj_model bin/obj_parser bin/shader \
	 	$(GENERAL_HELPER)/linear_math \
		$(GENERAL_HELPER)/gl_helper \
		$(GENERAL_HELPER)/general_helper \
	 	$(GENERAL_HELPER)/obj_parser \
	 	-lm -lstbi src/main.c \
	 	`pkg-config --libs $(PKG_CONFIG_LIBS)`

bin/obj_model: 
	gcc $(FLAGS) -c src/obj_model.c -o bin/obj_model 

bin/shader:
	gcc $(FLAGS) -c src/shader.c -o bin/shader 

bin/obj_parser: 
	gcc $(FLAGS) -c src/obj_parser.c -o bin/obj_parser

bin/camera:
	gcc $(FLAGS) -c src/camera.c -o bin/camera

clean:
	rm -f bin/*

debug:
	make clean
	make "BUILD=debug"

release:
	make clean
	make 
