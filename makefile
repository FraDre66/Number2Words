PROJ := Number2Words
OBJDIR := obj

OBJS = $(OBJDIR)/*.o
CPFLAGS=-Wall -Wextra
LDLIBS =
INCDIR = 
$(PROJ) : $(OBJS)
	cc -o $(PROJ) $(OBJS) $(LDLIBS)

$(OBJS) : *.c | $(OBJDIR)
	cc -c $(CPFLAGS) $< $(INCDIR) -o $@

$(OBJDIR) : 
	mkdir $(OBJDIR)

.PHONY: clean
clean: 
	rm -fr $(OBJDIR) $(PROJ)

.PHONY: run
run: $(PROJ)
	./$(PROJ)

