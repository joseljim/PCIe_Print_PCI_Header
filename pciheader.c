/*
MIT License

Copyright (c) 2022 José Luis Jiménez, Rodolfo Casillas and Erick Contreras

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
// ============================================================================
//  pciheader -- Prints the header of a PCI(e) device
//  Author(s): Jose Luis Jimenez, Rodolfo Casillas, Erick Contreras
//             A01229493         A01633245          A01630105
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/pci.h"

// TODO:
// Investigate the final address of the PCI header and complete the define below
#define PCI_HEADER_FINAL_ADDRESS 0x40

struct config_space_bitfield {
    char name[64];
    unsigned int offset;
    unsigned int size;
};

void print_pci_header(struct pci_dev *pdev);
struct pci_dev * search_device(struct pci_access *pacc, u8 bus, u8 slot, u8 func);
void int_2_hexstr(u32 value, unsigned int size, char *destination);
int convert_hexstring(char *hexstring);

// TODO:
// Explain what this struct is
// Explain what each column (field) of this struct represents

// This struct contains all the registers of a PCI type 0 header (endpoint)
// First column represents the registers, second column the offsets and the third column represents the size (in Bytes)

struct config_space_bitfield type_0_header[] = {
    {"Vendor ID",               0x0,    2},
    {"Device ID",               0x2,    2},
    {"Command",                 0x4,    2},
    {"Status",                  0x6,    2},
    {"Revision ID",             0x8,    1},
    {"Class Code",              0xA,    3},
    {"Cache Line S",            0xC,    1},
    {"Lat. Timer",              0xD,    1}, // Found error: Lat. Timer goes after Cache Line S, confirmed the mismatch with online documentation, so the order was fixed
    {"Header Type",             0xE,    1},
    {"BIST",                    0xF,    1},
    {"BAR 0",                   0x10,   4},
    {"BAR 1",                   0x14,   4},
    {"BAR 2",                   0x18,   4},
    {"BAR 3",                   0x1C,   4},
    {"BAR 4",                   0x20,   4},
    {"BAR 5",                   0x24,   4},
    {"Cardbus CIS Pointer",     0x28,   4},
    {"Subsystem Vendor ID",     0x2C,   2},
    {"Subsystem ID",            0x2E,   2},
    {"Expansion ROM Address",   0x30,   4},
    {"Cap. Pointer",            0x34,   1},
    {"Reserved",                0x35,   3},
    {"Reserved",                0x38,   4},
    {"IRQ",                     0x3C,   1},
    {"IRQ Pin",                 0x3D,   1},
    {"Min Gnt.",                0x3E,   1},
    {"Max Lat.",                0x3F,   1},
    {"End",                     0x40,   5},
};

// TODO:
// Explain what this struct is
// Explain what each column (field) of this struct represents

// This struct contains all the registers of a PCI type 1 header (Bridge)
// First column represents the registers, second column the offsets and the third column represents the size (in Bytes)

struct config_space_bitfield type_1_header[] = {
    {"Vendor ID",               0x0,    2},
    {"Device ID",               0x2,    2},
    {"Command",                 0x4,    2},
    {"Status",                  0x6,    2},
    {"Revision ID",             0x8,    1},
    {"Class Code",              0xA,    3},
    {"Cache Line S",            0xC,    1},
    {"Lat. Timer",              0xD,    1}, // Found error: Lat. Timer goes after Cache Line S, confirmed the mismatch with online documentation, so the order was fixed
    {"Header Type",             0xE,    1},
    {"BIST",                    0xF,    1},
    {"BAR 0",                   0x10,   4},
    {"BAR 1",                   0x14,   4},
    {"Primary Bus",             0x18,   1}, // Found error: Primary bus goes before Secondary Bus, confirmed the mismatch with online documentation, so the order was fixed
    {"Secondary Bus",           0x19,   1},
    {"Sub Bus",                 0x1A,   1},
    {"Sec Lat timer",           0x1B,   1},
    {"IO Base",                 0x1C,   1},
    {"IO Limit",                0x1D,   1},
    {"Sec. Status",             0x1E,   2},
    {"Memory Base",             0x20,   2}, //  Found error: Memory base goes before Memory limit, confirmed the mismatch with online documentation, so the order was fixed
    {"Memory Limit",            0x22,   2},
    {"Pref. Memory Base",       0x24,   2}, //  Found error: Pref. Memory base goes before Pref. Memory limit, confirmed the mismatch with online documentation, so the order was fixed
    {"Pref. Memory Limit",      0x26,   2}, 
    {"Pref. Memory Base U",     0x28,   4},
    {"Pref. Memory Base L",     0x2C,   4},
    {"IO Base Upper",           0x30,   2},
    {"IO Limit Upper",          0x32,   2},
    {"Cap. Pointer",            0x34,   1},
    {"Reserved",                0x35,   3},
    {"Exp. ROM Base Addr",      0x38,   4},
    {"IRQ Line",                0x3C,   1},
    {"IRQ Pin",                 0x3D,   1},
    {"Min Gnt.",                0x3E,   1},
    {"Max Lat.",                0x3F,   1},
    {"End",                     0x40,   5},
};

struct config_space_bitfield *types[2] = {&type_0_header[0], &type_1_header[0]}; // pointer to both types of headers (endpoint and bridges)

// function to print the pci header once all the information is obtained
void print_pci_header(struct pci_dev *pdev) {
    u8 header_type = 0;
    u32 value;
    u32 bitfield_value;
    u64 mask;
    unsigned int pci_header_address;
    unsigned int space_available;
    unsigned int padding;
    unsigned int bitfield = 0;
    unsigned int bitfield_copy;
    int j;
    struct config_space_bitfield *ptr;
    char str_value[16];
    const char *ctypes[] = {"n Endpoint", " Bridge"};

    if(pdev == NULL){ // exit if no device is given as parameter
        return;
    }

    // TODO:
    // Explain what the following code does

    // This function retrieves the type of PCI device we are delaing with, either endpoint or bridge,
    // by using the mask PCI_HEADER_TYPE which determines the position where this information is available
    header_type = pci_read_byte(pdev, PCI_HEADER_TYPE) & 0x1;
    // here we hold a pointer to the corresponding type of pci device
    ptr         = types[header_type];

    // TODO:
    // Print the following message:
    // Selected device <bus>:<device>:<function> is a <type>
    // Substitute <bus> <device> and <function> with the values of bus, device and function, respectively
    // Substitute <type> with either Endpoint or Bridge, accordingly
    printf("Selected device %x:%x:%x is a%s\n", pdev->bus, pdev->dev, pdev->func, ctypes[header_type]);

    printf("|-----------------------------------------------------------|\t\t|-----------------------------------------------------------|\n");
    printf("|    Byte 0    |   Byte 1     |    Byte 2    |    Byte 3    |\t\t|    Byte 0    |   Byte 1     |    Byte 2    |    Byte 3    |\n");
    printf("|-----------------------------------------------------------|\t\t|-----------------------------------------------------------|\tAddress\n");
    for(pci_header_address=0; pci_header_address<PCI_HEADER_FINAL_ADDRESS; pci_header_address+=4){ // iterate through all the registers of the header
        bitfield_copy = bitfield;

        putchar('|');

        while(ptr[bitfield].offset < pci_header_address+4){ 
            space_available = 14 * ptr[bitfield].size + (ptr[bitfield].size -1);
            padding = (space_available - strlen(ptr[bitfield].name)) / 2;

            for(j=0; j<(int) padding; j++){
                putchar(' ');
            }

            // TODO:
            // Print name of PCI configuration register
            // printf("%s", /*???*/);
            printf("%s", ptr[bitfield].name);

            for(j=(int) padding + strlen(ptr[bitfield].name); j<(int) space_available; j++){
                putchar(' ');
            }

            putchar('|');
            bitfield++;
        }

        // TODO:
        // Explain what this function does

        // This function retrieves the value of the corresponding PCI register using the corresponding address
        // the long option is used since not all register contain information with size of a byte
        value = pci_read_long(pdev, pci_header_address);

        bitfield = bitfield_copy;
        printf("\t\t|");

        while(ptr[bitfield].offset < pci_header_address+4){

            if(ptr[bitfield].size == 5){
                break;
            }

            // TODO: 
            // Explain the purpose of the following line
            
            // This line generates a mask according to the number of bits that are present in each register of the PCI header,
            // so that all the information of the register is obtained
            mask = ((1L<<(ptr[bitfield].size * 8))-1) << (8*(ptr[bitfield].offset - pci_header_address));

            bitfield_value = (value & mask) >> (8*(ptr[bitfield].offset - pci_header_address));

            space_available = 14 * ptr[bitfield].size + ptr[bitfield].size -1;
            padding = (space_available - ( 2 + ptr[bitfield].size)) / 2;

            for(j=0; j<(int) padding; j++){
                putchar(' ');
            }

            int_2_hexstr(bitfield_value, ptr[bitfield].size, str_value);
            printf("%s", str_value);

            for(j=(int) padding+strlen(str_value); j<(int) space_available; j++){
                putchar(' ');
            }
            putchar('|');
            bitfield++;
        }
        printf("\t0x%02x", pci_header_address);
        printf("\n|-----------------------------------------------------------|\t\t|-----------------------------------------------------------|\n");
    }
}

// function to access the PCI device depending on the arguments bus, slot and function
struct pci_dev * search_device(struct pci_access *pacc, u8 bus, u8 slot, u8 func) {
    struct pci_dev *dev;
    for(dev = pacc->devices; dev != NULL; dev = dev->next){
        if((dev-> bus == bus) && (dev->dev == slot) && (dev->func == func)){
            return dev;
        }
    }
    return NULL;
}

// function to convert an int to a string representing a hex value
void int_2_hexstr(u32 value, unsigned int size, char *destination) {
    const char letters[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    unsigned int i;

    /* Init string */
    strcpy(destination, "0x");
    for(i=0; i<size; i++){
        strcat(destination, "00");
    }
    i=2+2*size - 1;

    while((value > 0) && (i>1)){
        destination[i] = letters[(value & 0xf)];
        value = value >> 4;
        i--;
    }
}

// function to convert a string representign a number to an integer
int convert_hexstring(char *hexstring) {
    int number;
    if(strstr(hexstring, "0x") == NULL)
        number = (int) strtol(hexstring, NULL, 16);
    else
        number = (int) strtol(hexstring, NULL, 0);
    return number;
}

int main(int argc, char *argv[])
{
  struct pci_access *pacc;
  struct pci_dev    *dev;
  u8 bus;
  u8 slot;
  u8 func;

  // Check arguments
  // Verify that all 4 arguments are passed to the program
  // 1st arg is the name of the program
  // 2nd arg is Bus number to search for
  // 3rd arg is Device number to search for within the bus
  // 4th arg is Function number to search for within the device
  if(argc != 4){
      printf("Three Arguments must be passed!\n");
      printf("Usage: %s [bus] [device] [function]\n", argv[0]);
      printf("With:\n");
      printf("\tbus:\tBus number of device to print PCI Header\n");
      printf("\tdevice:\tDevice number of device to print PCI Header\n");
      printf("\tbus:\tFunction number of device to print PCI Header\n");
      return -1;
  }
  // TODO:
  // Explain what the following function calls do

  // pci_alloc() of struct pci_access hold necessary information for the access that is going to done to the PCI device 
  pacc = pci_alloc();
  // pci_init() initializes said access
  pci_init(pacc);
  // pci_scan_bus() scans the device that was previously accessed with the configurations made by the two functions above
  pci_scan_bus(pacc);

  // parse user input
  bus  = convert_hexstring(argv[1]);
  slot = convert_hexstring(argv[2]);
  func = convert_hexstring(argv[3]);

  // TODO: 
  // Complete the function call for searching the device provided by the user
  //dev = search_device(/*???*/, /*???*/, /*???*/, /*???*/,);
  dev = search_device(pacc, bus, slot, func);

  if(dev == NULL) {
      printf("No device found with %x:%x:%x\n", bus, slot, func); // error check if device is not found
      return -1;
  }
  
  // pci_fill_info returns the values of the known fields of the PCI device
  pci_fill_info(dev,PCI_FILL_IDENT | PCI_FILL_CLASS);
  print_pci_header(dev); // print PCI header
  
  // pci_cleanup() terminates the access previously initiated once everything is complete
  pci_cleanup(pacc);
  return 0;
}
