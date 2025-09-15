# Name of final binary/executable
TARGET := Number2Words 
# folder to generate the output (*.o/*.d)
OBJ_DIR := obj
# folder where to find the sources (*.c/*.h)
SRC_DIR := src
CFLAGS  :=-Wall -Wextra 
# external include directories (-I /foo/) leave empty if not required:
INC	:= 
# external libraries (leave empty if not required):
LDLIBS = 
#keep empty, only required if the executable needs admin rights to run 
SUDO = 
## DON'T CHANGE BELOW THIS LINE, SHOULD WORK IN GENERIC WAY ##
SRCS    := $(wildcard $(SRC_DIR)/*.c)
OBJS    := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
DEPS    := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.d, $(SRCS))
$(TARGET) : $(OBJS)
	@echo Linking $@ ...
	cc -o $(TARGET) $(OBJS) $(LDLIBS)

# Include generated dependencies, so changes in header file(s) will
# trigger to build dependend source files with next make. 
-include $(DEPS)

# Rule to compile the sources from SRC_DIR, write the objects to
# OBJ_DIR and generate the dependencies files (*.d)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR) 
	@echo === Building $@ === 
	cc -c $(CFLAGS) $(INC) -MMD -MD $< -o $@

# Generates the OBJ_DIR if not yet existing
$(OBJ_DIR):
	mkdir $(OBJ_DIR)

# clean: remove all generated files/directories
.PHONY: clean
clean: 
	rm -fr $(OBJ_DIR) $(TARGET)
	
# debug: rebuild all from scratch with debug information
.PHONY: debug
debug: CFLAGS += -ggdb
debug: clean $(TARGET)
	@echo === Built with debug information now start debugger ===
	sudo killall $(TARGET) 2> /dev/null || true
	sudo gdb $(TARGET) 

# install: copy the current binary to /usr/local/bin
.PHONY: install
install: clean $(TARGET)
	@echo === Install $@ ===
	sudo killall $(TARGET) 2> /dev/null || true
	sudo cp $(TARGET) /usr/local/bin
	sudo cp $(TARGET).ini /usr/local/bin
	sudo chmod 744 /usr/local/bin/$(TARGET)
	sudo chmod 644 /usr/local/bin/$(TARGET).ini

# start the application
.PHONY: run
run: $(TARGET)
	sudo killall $(TARGET) 2> /dev/null || true
	sudo ./$(TARGET)
