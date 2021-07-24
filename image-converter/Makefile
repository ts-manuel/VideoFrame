
# Variables
OBJS = main.c stb_image.c array.c converter.c

# Default target
release: $(OBJS)
	gcc -Wall -o imgconv $(OBJS) -lm

debug: $(OBJS)
	gcc -Wall -o imgconv $(OBJS) -lm -DDEBUG