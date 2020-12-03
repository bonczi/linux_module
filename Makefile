obj-m += my_key.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -o app user_app.c
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
