# binaries file names
# each binary source code is expected to be in $(SRCDIR)/<binary>
TARGET      := main

# makefile parameters
SRCDIR      := src
LIBSDIR     := libs
TESTDIR     := tests
BUILDDIR    := int
TARGETDIR   := target
SRCEXT      := cpp

# compiler parameters
CC          := g++
CFLAGS      := -g3 -std=c++14 -Wall -Wpedantic -Werror -pg
LIB         := m
INC         := /usr/local/include libs
DEFINES     :=


#---------------------------------------------------------------------------------
# DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

# sets the src directory in the VPATH
VPATH := $(SRCDIR)

# sets the build directory based on the binary
BUILDDIR := $(BUILDDIR)/$(BIN)

# source files
SRCS := $(shell find $(SRCDIR)/$(BIN) -type f -name *.$(SRCEXT))
SRCS += $(shell find $(LIBSDIR) -type f -name *.$(SRCEXT))

# object files
OBJS := $(patsubst %,$(BUILDDIR)/%,$(SRCS:.$(SRCEXT)=.o))

# includes the flag to generate the dependency files when compiling
CFLAGS += -MD


# special definitions used for the unit tests
ifeq ($(MAKECMDGOALS),$(TARGETDIR)/tests)
    # adds an extra include so the tests can include the sources
	INC += src

	# sets the special define for tests
	DEFINES := __TESTS__ $(DEFINES)

	# includes the tests directory in the VPATH
	VPATH := $(TESTDIR) $(VPATH)

	# test sources
	TEST_SRCS := $(shell find $(TESTDIR)/$(BIN) -type f -name *.$(SRCEXT))

	# test objects
	OBJS := $(patsubst %,$(BUILDDIR)/%,$(TEST_SRCS:.$(SRCEXT)=.o)) $(OBJS)
endif

# adds the include prefix to the include directories
INC := $(addprefix -I,$(INC))

# adds the lib prefix to the libraries
LIB := $(addprefix -l,$(LIB))

# adds the define prefix to the defines
DEFINES := $(addprefix -D,$(DEFINES))


# default: compiles all the binaries
all: $(addprefix bin-,$(TARGET))

# compiles the binary
bin-%:
	@$(MAKE) $(TARGETDIR)/$* BIN=$*

# compiles and runs the unit tests
tests:
	@$(MAKE) $(TARGETDIR)/tests BIN=tests

# shows usage
help:
	@echo "To compile all binaries:"
	@echo
	@echo "\t\033[1;92m$$ make\033[0m"
	@echo
	@echo "To compile just one binary:"
	@echo
	@echo "\t\033[1;92m$$ make bin-\033[1;31m<name>\033[0m"
	@echo
	@echo "where \033[1;31m<name>\033[0m is the name of the binary to compile."
	@echo "Source files for each binary are expected to be in \033[1;92m$(SRCDIR)/\033[0m\033[1;31m<name>\033[0m."
	@echo "Additional source files in \033[1;92m$(LIBSDIR)\033[0m are available for every binary."
	@echo
	@echo "Compiled binaries can be found in \033[1;92m$(TARGETDIR)\033[0m."
	@echo

# clean objects and binaries
clean:
	@$(RM) -rf $(BUILDDIR) $(TARGETDIR)

# creates the directories
dirs:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

# INTERNAL: builds the binary
$(TARGETDIR)/$(BIN): $(OBJS) | dirs
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) $^ $(LIB) -o $@
	@echo "LD $@"

# INTERNAL: builds and runs the unit tests
$(TARGETDIR)/tests: $(OBJS) | dirs
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) $^ $(LIB) -o $(TARGETDIR)/tests
	@echo "LD $@"
	./$(TARGETDIR)/tests

# rule to build object files
$(BUILDDIR)/%.o: %.$(SRCEXT)
	@mkdir -p $(basename $@)
	@echo "CC $<"
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) $(LIB) -c -o $@ $<


.PHONY: clean dirs tests all

# includes generated dependency files
-include $(OBJS:.o=.d)
