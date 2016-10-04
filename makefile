ifeq ($(BUILD),debug)
	FLAGS = -Og -Wall -g -DDEBUG
else
	FLAGS = -O3 -Wall -Werror -s -DNDEBUG
endif

GENERAL_HELPER = ~/programming_projects/c/general/bin

#bin/main : bin/main_window bin/obj_model bin/obj_parser bin/linear_math bin/gl_helper bin/camera
#	gcc $(FLAGS) `pkg-config --cflags gtk+-3.0 glew assimp` \
#-o bin/main bin/main_window bin/obj_model bin/obj_parser bin/linear_math \
#$(GENERAL_HELPER)/general_helper \
#bin/camera bin/gl_helper -lstbi -lm src/main.c `pkg-config --libs gtk+-3.0 glew assimp`
#
#bin/main_window : bin/obj_model
#	gcc $(FLAGS) `pkg-config --cflags gtk+-3.0 glew ` -c src/main_window.c -o bin/main_window \
#`pkg-config --libs gtk+-3.0 glew`

bin/main: bin/obj_model bin/obj_parser bin/camera bin/shader
	gcc $(FLAGS) `pkg-config --cflags glfw3 glew glib-2.0 assimp` \
	 	-o bin/main bin/obj_model bin/obj_parser bin/shader \
	 	$(GENERAL_HELPER)/linear_math $(GENERAL_HELPER)/gl_helper $(GENERAL_HELPER)/general_helper \
	 	$(GENERAL_HELPER)/obj_parser \
	 	-lm -lstbi src/main.c \
	 	`pkg-config --libs glfw3 glew glib-2.0 assimp`

bin/obj_model: 
	gcc $(FLAGS) `pkg-config --cflags glib-2.0` -c src/obj_model.c -o bin/obj_model `pkg-config --libs glib-2.0`

bin/shader:
	gcc $(FLAGS) `pkg-config --cflags glib-2.0` -c src/shader.c -o bin/shader `pkg-config --libs glib-2.0`

bin/obj_parser: 
	gcc $(FLAGS) -c src/obj_parser.c -o bin/obj_parser

bin/camera:
	gcc $(FLAGS) -c src/camera.c -o bin/camera

#bin/gl_helper:
#	gcc -c src/gl_helper.c -o bin/gl_helper
#
#bin/linear_math:
#	gcc -c src/linear_math.c -o bin/linear_math

clean:
	if [ $(shell find 'bin' -type d -empty) ]; then\
		echo 'bin is clean';\
	else\
		echo 'cleaning bin ...';\
		rm -r bin/*;\
	fi

debug:
	make clean
	make "BUILD=debug"

release:
	make clean
	make 
