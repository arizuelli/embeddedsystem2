#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Per la funzione sleep
#include <fcntl.h>
#include <linux/i2c-dev.h>

#define I2C_BUS "/dev/i2c-1"
#define TCS34725_ADDR 0x29

#define TCS34725_ENABLE_REG 0x80
#define TCS34725_ATIME_REG 0x81
#define TCS34725_WTIME_REG 0x83
#define TCS34725_CONTROL_REG 0x8F
#define TCS34725_DATA_REG 0x94

int main() 
{
    int file;
    char *bus = I2C_BUS;
    if ((file = open(bus, O_RDWR)) < 0) 
    {
        perror("Failed to open the bus");
        exit(1);
    }
    
    if (ioctl(file, I2C_SLAVE, TCS34725_ADDR) < 0)
    {
        perror("Failed to acquire bus access and/or talk to slave");
        exit(1);
    }
    
    // Configurazione del sensore
    char config[2];
    config[0] = TCS34725_ENABLE_REG;
    config[1] = 0x03; // Power ON, RGBC enable, wait time disable
    write(file, config, 2);
    
    config[0] = TCS34725_ATIME_REG;
    config[1] = 0x00; // Atime = 700 ms
    write(file, config, 2);
    
    config[0] = TCS34725_WTIME_REG;
    config[1] = 0xFF; // WTIME : 2.4ms
    write(file, config, 2);
    
    config[0] = TCS34725_CONTROL_REG;
    config[1] = 0x00; // AGAIN = 1x
    write(file, config, 2);
    
    sleep(1); // Attendiamo 1 secondo prima di leggere i dati
    
    // Lettura dei dati dal sensore
    char reg = TCS34725_DATA_REG;
    if(write(file, &reg, 1) != 1)
    {
        perror("Failed to write to the i2c bus");
        exit(1);
    }
    
    char data[8];
    if(read(file, data, 8) != 8)
    {
        perror("Failed to read from the i2c bus");
        exit(1);
    }
    
    // Convertiamo i dati
    int cData = (data[1] << 8) | data[0];
    int red = (data[3] << 8) | data[2];
    int green = (data[5] << 8) | data[4];
    int blue = (data[7] << 8) | data[6];
    
    // Calcoliamo la luminosità
    float luminance = (-0.32466) * red + (1.57837) * green + (-0.73191) * blue;
    luminance = luminance < 0 ? 0 : luminance; // Assicuriamoci che la luminosità non sia negativa
    
    // Stampiamo i dati
    printf("Red color luminance: %d lux\n", red);
    printf("Green color luminance: %d lux\n", green);
    printf("Blue color luminance: %d lux\n", blue);
    printf("IR luminance: %d lux\n", cData);
    printf("Ambient Light Luminance: %.2f lux\n", luminance);
    
    // Chiudiamo il file I2C
    close(file);
    
    return 0;
}

