CC=gcc
CFLAGS=-O3 -Wall -Wextra -Werror -DNDEBUG
DFLAGS=-Og -Wall -Wextra -Werror

# compiled target name
TARGET_NAME := lop


# define executable directory
TARGET_DIR			:=	build
# define output directory
OBJECT_DIR			:=	bin
# define source directory 
SOURCE_DIR			:=	src
# define include directory
INCLUDE_DIR			:=	include

#directory tree for bin directory.
DIRS = $(shell find src -type d | sed 's/src/./g')


#src/.../%.c -> bin/.../$.o
G_OBJS = $(patsubst src/%.c,bin/%.o,$(shell find src -type f -name "*.c"))

#src/.../%.c -> bin/.../$_debug.o
G_DOBJS = $(patsubst src/%.c,bin/%_debug.o,$(shell find src -type f -name "*.c"))

# compile target
lop: $(G_OBJS) buildrepo
	$(CC) $(CFLAGS) $(G_OBJS) -o ${TARGET_DIR}/${TARGET_NAME}

# compile target with debug flags
debug: $(G_DOBJS) buildrepo
		$(CC) $(DFLAGS) $(G_DOBJS) -o ${TARGET_DIR}/${TARGET_NAME}_debug

.PHONY: all
all: lop debug

.PHONY: default
default: all


# compile objects
bin/%.o: src/%.c buildrepo
	$(CC) $(CFLAGS) -c $< -I ${INCLUDE_DIR}  -o $@

# build objects with debug flags
bin/%_debug.o: src/%.c buildrepo
	$(CC) $(DFLAGS) -c $< -I ${INCLUDE_DIR}  -o $@



.PHONY:buildrepo
buildrepo:
	mkdir -p $(TARGET_DIR) $(OBJECT_DIR) $(INCLUDE_DIR)
	for dir in $(DIRS); do mkdir -p bin/$$dir; done


.PHONY: clean
clean:
	rm bin -Rf

.PHONY: cleaner
cleaner: clean
	rm build -Rf
