#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

#define SPI_DEVICE "/dev/spidev1.0"
#define SPI_MODE SPI_MODE_0
#define SPI_BITS_PER_WORD 8
#define SPI_SPEED_HZ 1000000 // 1MHz


#define GPIO_PATH "/sys/class/gpio"
#define SPI1_CS2 "48"   // Change this to the GPIO pin you want to control


int spi_fd;


void spi_init() {
    spi_fd = open(SPI_DEVICE, O_RDWR);
    
    if (spi_fd == -1) {
        perror("Error opening SPI device");
        exit(EXIT_FAILURE);
    }

    // Set SPI mode
    uint8_t mode = SPI_MODE;
    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) == -1) {
        perror("Error setting SPI mode");
        close(spi_fd);
        exit(EXIT_FAILURE);
    }

    // Set bits per word
    uint8_t bitsperword = SPI_BITS_PER_WORD;
    if (ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bitsperword) == -1) {
        perror("Error setting SPI bits per word");
        close(spi_fd);
        exit(EXIT_FAILURE);
    }

    // Set SPI speed
    uint32_t speed = SPI_SPEED_HZ;
    if (ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) {
        perror("Error setting SPI speed");
        close(spi_fd);
        exit(EXIT_FAILURE);
    }
}

void spi_transmit(uint16_t data) {
    uint8_t tx_buf[2];

    // Split 16-bit data into two 8-bit values
    tx_buf[0] = (data >> 8) & 0xFF;
    tx_buf[1] = data & 0xFF;

    struct spi_ioc_transfer spi_msg = {
        .tx_buf = (unsigned long)tx_buf,
        .len = sizeof(tx_buf),
    };

    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi_msg) == -1) {
        perror("Error transmitting data over SPI");
        close(spi_fd);
        exit(EXIT_FAILURE);
    }
}

uint16_t spi_receive() {

    uint8_t rx_buf[2];
    uint16_t received_data;

    struct spi_ioc_transfer spi_msg = {
        .rx_buf = (unsigned long)rx_buf,
        .len = sizeof(rx_buf),
    };

    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &spi_msg) == -1) {
        perror("Error receiving data over SPI");
        close(spi_fd);
        exit(EXIT_FAILURE);
    }
    
  

    // Combine two 8-bit values into a single 16-bit value
    received_data = (rx_buf[0] << 8) | rx_buf[1];

    return received_data;
}


// Function to export a GPIO pin
void export_gpio(const char *gpio_pin) {
    FILE *export_file = fopen(GPIO_PATH "/export", "w");
    if (export_file == NULL) {
        perror("Error exporting GPIO");
        exit(EXIT_FAILURE);
    }

    fprintf(export_file, "%s", gpio_pin);
    fclose(export_file);
}

// Function to set the direction of a GPIO pin
void set_gpio_direction(const char *gpio_pin, const char *direction) {
    char path[255];
    sprintf(path, GPIO_PATH "/gpio%s/direction", gpio_pin);

    FILE *direction_file = fopen(path, "w");
    if (direction_file == NULL) {
        perror("Error setting GPIO direction");
        exit(EXIT_FAILURE);
    }

    fprintf(direction_file, "%s", direction);
    fclose(direction_file);
}

// Function to set a GPIO pin high
void set_gpio_high(const char *gpio_pin) {
    char path[255];
    sprintf(path, GPIO_PATH "/gpio%s/value", gpio_pin);

    FILE *value_file = fopen(path, "w");
    if (value_file == NULL) {
        perror("Error setting GPIO high");
        exit(EXIT_FAILURE);
    }

    fprintf(value_file, "1");
    fclose(value_file);
}

// Function to set a GPIO pin low
void set_gpio_low(const char *gpio_pin) {
    char path[255];
    sprintf(path, GPIO_PATH "/gpio%s/value", gpio_pin);

    FILE *value_file = fopen(path, "w");
    if (value_file == NULL) {
        perror("Error setting GPIO low");
        exit(EXIT_FAILURE);
    }

    fprintf(value_file, "0");
    fclose(value_file);
}

// Function to unexport a GPIO pin
void unexport_gpio(const char *gpio_pin) {
    FILE *unexport_file = fopen(GPIO_PATH "/unexport", "w");
    if (unexport_file == NULL) {
        perror("Error unexporting GPIO");
        exit(EXIT_FAILURE);
    }

    fprintf(unexport_file, "%s", gpio_pin);
    fclose(unexport_file);
}


int main() {
    spi_init();
    
    float Voltage;
    float divide_Fact = 32768; // (2^15) half of 65536
    
    
    // Export the GPIO pin
    export_gpio(SPI1_CS2);

    // Set the GPIO direction to out
    set_gpio_direction(SPI1_CS2, "out");
    
    
    // uint16_t data_to_send = 0xf0f0;
    // Transmit data
    // spi_transmit(data_to_send);
    
    // Example: Transmit and receive 16-bit data continuously
    
    while (1) {
            
        // Set GPIO low
        set_gpio_low(SPI1_CS2);
  
        // Receive data
        uint16_t received_data = spi_receive();
        
        set_gpio_high(SPI1_CS2);
        usleep(500000);  // Sleep for 500ms (0.5 seconds)

        Voltage = (received_data - divide_Fact) * 0.000375; 
        
        printf("Calculated Voltage: %f \n", Voltage);
        usleep(1000000); // Sleep for 100ms (adjust as needed)
        
       
    }
// Unexport the GPIO pin
    unexport_gpio(SPI1_CS2);
    close(spi_fd);
    return 0;
}
