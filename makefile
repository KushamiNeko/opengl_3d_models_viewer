bin/main : bin/main_window bin/obj_model bin/obj_parser bin/linear_math bin/gl_helper bin/camera
	gcc `pkg-config --cflags gtk+-3.0 glew assimp` \
-o bin/main bin/main_window bin/obj_model bin/obj_parser bin/linear_math \
~/programming_projects/c/general/bin/general_helper \
bin/camera bin/gl_helper -lstbi -lm src/main.c `pkg-config --libs gtk+-3.0 glew assimp`

bin/main_window : bin/obj_model
	gcc `pkg-config --cflags gtk+-3.0 glew ` -c src/main_window.c -o bin/main_window \
`pkg-config --libs gtk+-3.0 glew`


bin/obj_model: bin/obj_parser bin/gl_helper
	gcc  -c src/obj_model.c -o bin/obj_model

bin/obj_parser: bin/linear_math
	gcc  -c src/obj_parser.c -o bin/obj_parser

bin/camera:
	gcc  -c src/camera.c -o bin/camera

bin/gl_helper:
	gcc -c src/gl_helper.c -o bin/gl_helper

bin/linear_math:
	gcc -c src/linear_math.c -o bin/linear_math

clean:
	if [ $(shell find 'bin' -type d -empty) ]; then\
		echo 'bin is clean';\
	else\
		echo 'cleaning bin ...';\
		rm -r bin/*;\
	fi

build:
	make clean
	make
