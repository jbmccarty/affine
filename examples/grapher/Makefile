OBJS := main.o grapher.o

grapher: $(OBJS)
	g++ `gtkmm-config --libs` $^ -o $@
$(OBJS): grapher.h
#grapher.cpp: plot.h
%.o: %.cpp
	g++ -O2 -c `gtkmm-config --cflags` $<
