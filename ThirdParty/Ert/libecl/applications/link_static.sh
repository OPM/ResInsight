gcc -o summary.x_static view_summary.o -L../lib -L../../libutil/lib -lecl -lutil -lm -lpt 
gcc -static -o summary.x_full_static view_summary.o -L../lib -L../../libutil/lib -lecl -lutil -lm -lpthread -lz
