# PCIe - Print the PCI Header (11/2021)
User application for reading the Peripheral Component Interconnect (PCI) header located in the PCI’s configuration space of a PCIe device.

## Compiling code

In a Linux terminal, input the following commands in order to compile the application. 
1. ```make clean```
2. ```make pciheader```

After successfully compiling the user application, you might be able to run the compiled program 
by simply typing ```./pciheader```. Please note that you also need to provide some arguments when executing this command. 

In order to obtain the arguments to run the program, in a Linux terminal type ```lspci```

You will obtain a list of the PCI devices like this:

![Screen Shot 2022-03-28 at 13 12 18](https://user-images.githubusercontent.com/78834111/160469838-bdfed10b-ae89-4309-b0f2-f548a22dd901.png)<br />
**Fig. 1:** List of the PCI devices.

To run the program type ```./pciheader <Bus number> <Device number> <Function number> ```

* The 2nd arg is Bus number to search for
* The 3rd arg is Device number to search for within the bus
* The 4th arg is Function number to search for within the device

## Example 

```$ ./pciheader 0 01 0 ```

Using the parameters from the second device displayed in **Fig. 1**

![Screen Shot 2022-03-28 at 13 10 32](https://user-images.githubusercontent.com/78834111/160469511-2e1f9f94-8206-409f-ace2-b977f2089f7f.png)<br />
**Fig. 2:** Example with a bridge device.

Project demostration [Youtube video](https://youtu.be/163qVFvFtpY)
