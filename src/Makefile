CC=$(CROSS_COMPILE)gcc
OBJS := main.o
combinationlock: $(OBJS)
	$(CC) -g -o combinationlock $(OBJS) -l wiringPi
$(OBJS) : %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@
