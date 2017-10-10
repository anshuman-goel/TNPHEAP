rm *.log
sudo rmmod npheap
sudo rmmod tnpheap
cd ../CSC501_NPHEAP/kernel_module/
make
sudo make install
cd ../library
make
sudo make install
cd ../../CSC501_TNPHEAP/kernel_module
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
