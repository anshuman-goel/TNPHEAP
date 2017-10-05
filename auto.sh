sudo rmmod npheap
sudo rmmod tnpheap
cd kernel_module
make
sudo make install
sudo insmod ../NPHeap/npheap.ko
sudo chmod 777 /dev/npheap
sudo insmod tnpheap.ko
sudo chmod 777 /dev/tnpheap
cd ../library
make
sudo make install
cd ../benchmark
make
