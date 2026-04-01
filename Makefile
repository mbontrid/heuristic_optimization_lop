CC=gcc
CFLAGS=-O3 -Wall -DNDEBUG #-Werror -Wextra 
DFLAGS=-Og -Wall #-Werror -Wextra 

# compiled target name
TARGET_NAME := lop


# define executable directory
TARGET_DIR			:=	bin
# define output directory
OBJECT_DIR			:=	build
# define source directory 
SOURCE_DIR			:=	src
# define include directory
INCLUDE_DIR			:=	include


#directory tree for OBJECT_DIR directory.
DIRS = $(shell find ${SOURCE_DIR} -type d | sed 's/${SOURCE_DIR}/./g')


#${SOURCE_DIR}/.../%.c -> ${OBJECT_DIR}/.../$.o
G_OBJS = $(patsubst ${SOURCE_DIR}/%.c,${OBJECT_DIR}/%.o,$(shell find ${SOURCE_DIR} -type f -name "*.c"))

#${SOURCE_DIR}/.../%.c -> ${OBJECT_DIR}/.../$_debug.o
G_DOBJS = $(patsubst ${SOURCE_DIR}/%.c,${OBJECT_DIR}/%_debug.o,$(shell find ${SOURCE_DIR} -type f -name "*.c"))

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
${OBJECT_DIR}/%.o: ${SOURCE_DIR}/%.c buildrepo
	$(CC) $(CFLAGS) -c $< -I ${INCLUDE_DIR}  -o $@

# build objects with debug flags
${OBJECT_DIR}/%_debug.o: ${SOURCE_DIR}/%.c buildrepo
	$(CC) $(DFLAGS) -c $< -I ${INCLUDE_DIR}  -o $@



.PHONY:buildrepo
buildrepo:
	mkdir -p $(TARGET_DIR) $(OBJECT_DIR) $(INCLUDE_DIR)
	for dir in $(DIRS); do mkdir -p ${OBJECT_DIR}/$$dir; done


.PHONY: clean
clean:
	rm ${OBJECT_DIR} -Rf

.PHONY: cleaner
cleaner: clean
	rm ${TARGET_DIR} -Rf
