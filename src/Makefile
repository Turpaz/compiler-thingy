CPP :=g++
TARGET :=../bin/lang.exe # change this to the name of your language
LDFLAGS :=
SOURCES := $(shell find . -name "*.cpp")
SOURCES += $(shell find . -name "*.hpp")

TEST_CODE :=../test/base.lng # change the extension to match your language
TEST_TARGET :=../test/base.exe

all:
	$(CPP) $(LDFLAGS) $(CFLAGS) -o $(TARGET) $(SOURCES)

g:
	$(CPP) $(LDFLAGS) $(CFLAGS) -g -o $(TARGET) $(SOURCES)

db:
	gdb $(TARGET)

test:
	$(TARGET) $(TEST_CODE) -o $(TEST_TARGET) && $(TEST_TARGET)

clean:
	del $(TARGET)