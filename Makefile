
xxtea.so: xxtea.c
	gcc --shared -fPIC -O2 -o xxtea.so xxtea.c

clean:
	rm xxtea.so

