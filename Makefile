# binary file name
TARGET      := voley

# makefile parameters
SRCDIR      := src
TESTDIR     := tests
BUILDDIR    := int
TARGETDIR   := target
SRCEXT      := cpp

# compiler parameters
CC          := g++
CFLAGS      := -g3 -std=c++14 -Wall -Wpedantic -Werror -pg
LIB         := m
INC         := /usr/local/include
DEFINES     :=


#---------------------------------------------------------------------------------
# DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------

# sets the src directory in the VPATH
VPATH := $(SRCDIR)

# sets the build directory based on the profile
BUILDDIR := $(BUILDDIR)/$(PROFILE)

# source files
SRCS := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))

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
	TEST_SRCS := $(shell find $(TESTDIR) -type f -name *.$(SRCEXT))

	# test objects
	OBJS := $(patsubst %,$(BUILDDIR)/%,$(TEST_SRCS:.$(SRCEXT)=.o)) $(OBJS)
endif

# adds the include prefix to the include directories
INC := $(addprefix -I,$(INC))

# adds the lib prefix to the libraries
LIB := $(addprefix -l,$(LIB))

# adds the define prefix to the defines
DEFINES := $(addprefix -D,$(DEFINES))


# default: compiles the binary
$(TARGET):
	@$(MAKE) $(TARGETDIR)/$(TARGET) PROFILE=$(TARGET)

# compiles and runs the unit tests
tests:
	@$(MAKE) $(TARGETDIR)/tests PROFILE=tests

# shows usage
help:
	@echo "To compile the binary:"
	@echo
	@echo "\t\033[1;92mmake $(TARGET)\033[0m"
	@echo
	@echo "To compile and run the unit tests:"
	@echo
	@echo "\t\033[1;92mmake tests\033[0m"
	@echo

# clean objects and binaries
clean:
	@$(RM) -rf $(BUILDDIR) $(TARGETDIR)

# creates the directories
dirs:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

# INTERNAL: builds the binary
$(TARGETDIR)/$(TARGET): $(OBJS) | dirs
	@$(CC) $(CFLAGS) $(INC) $(DEFINES) $^ $(LIB) -o $(TARGETDIR)/$(TARGET)
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


.PHONY: clean dirs tests $(TARGET)

# includes generated dependency files
-include $(OBJS:.o=.d)
