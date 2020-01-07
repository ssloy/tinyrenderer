SYSCONF_LINK = g++
CPPFLAGS     = -Wall -Wextra -Weffc++ -pedantic -std=c++98 -MD
LDFLAGS      = -O3
LIBS         = -lm

DESTDIR = ./
TARGET  = main

DEPS=$(wildcard *.d)
OBJECTS := $(patsubst %.cpp,%.o,$(wildcard *.cpp))


all: $(DESTDIR)$(TARGET)

$(DESTDIR)$(TARGET): $(OBJECTS)
	$(SYSCONF_LINK) -Wall $(LDFLAGS) -o $(DESTDIR)$(TARGET) $(OBJECTS) $(LIBS)

$(OBJECTS): %.o: %.cpp
	$(SYSCONF_LINK) -Wall $(CPPFLAGS) -c $(CFLAGS) $< -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
	-rm -f *.tga
	-rm -f $(DEPS)

-include $(OBJECTS:.o=.d)
