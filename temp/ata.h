#include "tool.h"

// Corrected ATA Disk Operations
#define ATA_PRIMARY_IO  0x1F0
#define ATA_STATUS      (ATA_PRIMARY_IO + 7)
#define ATA_READY       0x08
#define ATA_DRQ         0x08  // Data Request bit
#define ATA_BSY         0x80  // Busy bit

void detect_disk() {
    uint8_t status = inio(ATA_STATUS);
    if (status == 0) {
        puts("No hard disk detected.\n");
    } else {
        puts("Hard disk detected!\n");
    }
}

int ata_read_sector(uint32_t lba, uint8_t *buffer) {
    printf("Reading LBA: %d\n", lba);

    // Select drive (Master, LBA mode)
    outio(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    io_wait();
    
    // Set up LBA addressing
    outio(ATA_PRIMARY_IO + 1, 0);           // Features register = 0
    outio(ATA_PRIMARY_IO + 2, 1);           // Sector count = 1
    outio(ATA_PRIMARY_IO + 3, lba & 0xFF);  // LBA low
    outio(ATA_PRIMARY_IO + 4, (lba >> 8) & 0xFF);  // LBA mid
    outio(ATA_PRIMARY_IO + 5, (lba >> 16) & 0xFF); // LBA high
    io_wait();
    
    // Send READ SECTORS command
    outio(ATA_PRIMARY_IO + 7, 0x20);
    io_wait();
    
    // Wait for BSY to clear and DRQ to set
    int timeout = 100000;
    uint8_t status;
    do {
        status = inio(ATA_STATUS);
        if (--timeout == 0) {
            printf("Error: Disk timeout waiting for data!\n");
            return -1;
        }
    } while ((status & ATA_BSY) || !(status & ATA_DRQ));
    
    //printf("Drive ready, reading data...\n");
    
    // Read the complete sector (256 words = 512 bytes)
    for (int i = 0; i < 256; i++) {
        uint16_t data = inio16(ATA_PRIMARY_IO);
        buffer[i * 2] = data & 0xFF;        // Low byte
        buffer[i * 2 + 1] = (data >> 8);    // High byte
        /*
        // Only print the first few values for debugging
        if (i < 5) {
            printf("[%d]: Read 0x%04X (%c%c)\n", i, data, 
                   (buffer[i * 2] >= 32 && buffer[i * 2] <= 126) ? buffer[i * 2] : '.',
                   (buffer[i * 2 + 1] >= 32 && buffer[i * 2 + 1] <= 126) ? buffer[i * 2 + 1] : '.');
        }
        */
    }
    
    printf("Data read complete!\n");
    return 0;
}

int ata_write_sector(uint32_t lba, const uint8_t *buffer) {
    printf("Writing LBA: %d\n", lba);
    
    // Select drive (Master, LBA mode)
    outio(ATA_PRIMARY_IO + 6, 0xE0 | ((lba >> 24) & 0x0F));
    io_wait();
    
    // Set up LBA addressing
    outio(ATA_PRIMARY_IO + 1, 0);           // Features register = 0
    outio(ATA_PRIMARY_IO + 2, 1);           // Sector count = 1 
    outio(ATA_PRIMARY_IO + 3, lba & 0xFF);  // LBA low
    outio(ATA_PRIMARY_IO + 4, (lba >> 8) & 0xFF);  // LBA mid
    outio(ATA_PRIMARY_IO + 5, (lba >> 16) & 0xFF); // LBA high
    io_wait();
    
    // Send WRITE SECTORS command
    outio(ATA_PRIMARY_IO + 7, 0x30);
    io_wait();
    
    // Wait for BSY to clear and DRQ to set
    int timeout = 100000;
    uint8_t status;
    do {
        status = inio(ATA_STATUS);
        if (--timeout == 0) {
            printf("Error: Disk timeout waiting for ready state!\n");
            return -1;
        }
    } while ((status & ATA_BSY) || !(status & ATA_DRQ));
    
    //printf("Writing data...\n");
    
    // Write the complete sector (256 words = 512 bytes)
    for (int i = 0; i < 256; i++) {
        uint16_t word = buffer[i * 2] | ((buffer[i * 2 + 1] << 8) & 0xFF00);
        outio16(ATA_PRIMARY_IO, word);
        /*
        // Only print the first few values for debugging
        if (i < 5) {
            printf("[%d]: Writing 0x%04X (%c%c)\n", i, word, 
                   (buffer[i * 2] >= 32 && buffer[i * 2] <= 126) ? buffer[i * 2] : '.',
                   (buffer[i * 2 + 1] >= 32 && buffer[i * 2 + 1] <= 126) ? buffer[i * 2 + 1] : '.');
        }
        */
    }
    
    // Wait for write to complete
    timeout = 100000;
    do {
        status = inio(ATA_STATUS);
        if (--timeout == 0) {
            printf("Error: Disk timeout after write!\n");
            return -1;
        }
    } while (status & ATA_BSY);
    
    printf("Write complete!\n");
    return 0;
}


void test_persistent_storage() {
    uint8_t buffer[512];
    
    detect_disk() ;
    
    // Initialize buffer with zeros
    memset(buffer, 0, 512);
    
    // Write test string
    strcpy((char*)buffer, "I am Abinesh!");
    
    /*
    // Debug Print Before Writing
    printf("Before writing (decimal values):\n");
    for (int i = 0; i < 32; i++) {
        printf("%d ", buffer[i]);  
    }
    printf("\n");
    
    printf("Write raw hex values:\n");
    for (int i = 0; i < 32; i++) {
        printf("%x ", buffer[i]);
    }
    printf("\n");
    */
    
    
    // Write to Disk
    if (ata_write_sector(1, buffer) != 0) {
        printf("Write failed!\n");
        return;
    }
    
    // Clear Buffer Before Reading
    memset(buffer, 0, 512);
    
    // Read from Disk
    if (ata_read_sector(1, buffer) != 0) {
        printf("Read failed!\n");
        return;
    }
    
    
     /*
    // Debug Print After Reading
    printf("After reading (decimal values):\n");
    for (int i = 0; i < 32; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");
    
    printf("Read raw hex values:\n");
    for (int i = 0; i < 32; i++) {
        printf("%x ", buffer[i]);
    }
    printf("\n");
    */
    
    // Print buffer as a string
    printf("Read from disk: %s\n", buffer);
}





