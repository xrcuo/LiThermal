/**
 * @file st7789_spi.c
 * ST7789 SPI interface implementation
 */

#include "st7789_spi.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <stdio.h>

/*********************
 * STATIC VARIABLES
 *********************/
static int spi_fd = -1;

/* GPIO文件描述符 */
static int gpio_dc_fd = -1;
static int gpio_rst_fd = -1;
static int gpio_cs_fd = -1;

/*********************
 * STATIC FUNCTIONS
 *********************/
static int gpio_export(int pin);
static int gpio_set_direction(int pin, const char *dir);
static int gpio_open_value_file(int pin);
static void gpio_set_value_fd(int fd, int value);
static void st7789_spi_transfer(uint8_t *tx, uint8_t *rx, int len);

/*********************
 * GPIO HELPER FUNCTIONS
 *********************/

/**
 * Export GPIO pin via sysfs
 */
static int gpio_export(int pin)
{
    FILE *f = fopen("/sys/class/gpio/export", "w");
    if (!f) {
        printf("[ST7789] ERROR: Cannot open /sys/class/gpio/export\n");
        return -1;
    }
    fprintf(f, "%d\n", pin);
    fclose(f);
    usleep(100000);  /* Wait 100ms for GPIO to be exported */
    return 0;
}

/**
 * Set GPIO direction (in/out)
 */
static int gpio_set_direction(int pin, const char *dir)
{
    char path[64];
    FILE *f;
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", pin);
    
    f = fopen(path, "w");
    if (!f) {
        printf("[ST7789] ERROR: Cannot open direction file for GPIO%d: %s\n", pin, path);
        return -1;
    }
    fprintf(f, "%s\n", dir);
    fclose(f);
    usleep(10000);  /* Wait 10ms */
    return 0;
}

/**
 * Open GPIO value file for faster I/O
 */
static int gpio_open_value_file(int pin)
{
    char path[64];
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", pin);
    return open(path, O_WRONLY);
}

/**
 * Write value to GPIO via file descriptor
 */
static void gpio_set_value_fd(int fd, int value)
{
    if (fd < 0) return;
    lseek(fd, 0, SEEK_SET);
    write(fd, value ? "1" : "0", 1);
}

/**
 * Initialize GPIO pins for ST7789
 */
static void st7789_gpio_init(void)
{
    printf("[ST7789] Initializing GPIO pins...\n");
    
    /* Export and configure DC pin (PB5 = GPIO37) */
    printf("[ST7789]   DC (GPIO%d)...\n", ST7789_SPI_DC_PIN);
    gpio_export(ST7789_SPI_DC_PIN);
    gpio_set_direction(ST7789_SPI_DC_PIN, "out");
    gpio_dc_fd = gpio_open_value_file(ST7789_SPI_DC_PIN);
    if (gpio_dc_fd < 0) printf("[ST7789] WARNING: DC pin open failed\n");
    
    /* Export and configure RST pin (PB4 = GPIO36) */
    printf("[ST7789]   RST (GPIO%d)...\n", ST7789_SPI_RST_PIN);
    gpio_export(ST7789_SPI_RST_PIN);
    gpio_set_direction(ST7789_SPI_RST_PIN, "out");
    gpio_rst_fd = gpio_open_value_file(ST7789_SPI_RST_PIN);
    if (gpio_rst_fd < 0) printf("[ST7789] WARNING: RST pin open failed\n");
    
    /* Export and configure CS pin (PD3 = GPIO101) - optional */
    printf("[ST7789]   CS (GPIO%d)...\n", ST7789_SPI_CS_PIN);
    gpio_export(ST7789_SPI_CS_PIN);
    gpio_set_direction(ST7789_SPI_CS_PIN, "out");
    gpio_cs_fd = gpio_open_value_file(ST7789_SPI_CS_PIN);
    if (gpio_cs_fd < 0) printf("[ST7789] WARNING: CS pin open failed\n");
    gpio_set_value_fd(gpio_cs_fd, 1);  /* CS默认高电平 */
    
    printf("[ST7789] GPIO initialization complete\n");
}

/*********************
 * GLOBAL FUNCTIONS
 *********************/

void st7789_spi_init(void)
{
    uint8_t mode = ST7789_SPI_MODE;
    uint8_t bits = 8;
    uint32_t speed = ST7789_SPI_SPEED_HZ;  /* 80MHz SPI clock */
    int ret;
    
    printf("[ST7789] Initializing SPI interface...\n");
    printf("[ST7789] GPIO Config: CS=%d DC=%d RST=%d BL=%d\n", 
           ST7789_SPI_CS_PIN, ST7789_SPI_DC_PIN, ST7789_SPI_RST_PIN, ST7789_SPI_BL_PIN);
    
    /* Initialize GPIO pins */
    st7789_gpio_init();
    
    /* Open SPI device */
    spi_fd = open(ST7789_SPI_BUS, O_RDWR);
    if (spi_fd < 0) {
        printf("[ST7789] ERROR: Failed to open SPI device: %s\n", ST7789_SPI_BUS);
        perror("open");
        return;
    }
    printf("[ST7789] SPI device opened: %s (fd=%d)\n", ST7789_SPI_BUS, spi_fd);
    
    /* Set SPI mode */
    ret = ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    if (ret < 0) {
        printf("[ST7789] ERROR: SPI set mode failed\n");
        perror("ioctl SPI_IOC_WR_MODE");
        close(spi_fd);
        return;
    }
    
    /* Set SPI bits per word */
    ret = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret < 0) {
        printf("[ST7789] ERROR: SPI set bits failed\n");
        perror("ioctl SPI_IOC_WR_BITS_PER_WORD");
        close(spi_fd);
        return;
    }
    
    /* Set SPI clock speed */
    ret = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret < 0) {
        printf("[ST7789] ERROR: SPI set speed failed\n");
        perror("ioctl SPI_IOC_WR_MAX_SPEED_HZ");
        close(spi_fd);
        return;
    }
    
    printf("[ST7789] SPI initialized successfully: %s @ %d Hz\n", ST7789_SPI_BUS, (int)speed);
}

void st7789_spi_write_cmd(uint8_t cmd)
{
    st7789_spi_set_dc(0);  /* Set DC to 0 for command */
    st7789_spi_transfer(&cmd, NULL, 1);
}

void st7789_spi_write_data(uint8_t data)
{
    st7789_spi_set_dc(1);  /* Set DC to 1 for data */
    st7789_spi_transfer(&data, NULL, 1);
}

void st7789_spi_write_data_array(uint8_t *data, uint16_t len)
{
    st7789_spi_set_dc(1);  /* Set DC to 1 for data */
    st7789_spi_transfer(data, NULL, len);
}

void st7789_spi_set_dc(int level)
{
    gpio_set_value_fd(gpio_dc_fd, level);
}

void st7789_spi_set_rst(int level)
{
    gpio_set_value_fd(gpio_rst_fd, level);
}

void st7789_spi_set_cs(int level)
{
    gpio_set_value_fd(gpio_cs_fd, level);
}

/*********************
 * STATIC FUNCTIONS
 *********************/

static void st7789_spi_transfer(uint8_t *tx, uint8_t *rx, int len)
{
    struct spi_ioc_transfer tr;
    int ret;
    
    if (spi_fd < 0) {
        printf("[ST7789] ERROR: SPI device not initialized\n");
        return;
    }
    
    memset(&tr, 0, sizeof(tr));
    tr.tx_buf = (unsigned long)tx;
    tr.rx_buf = (unsigned long)rx;
    tr.len = len;
    tr.delay_usecs = 0;
    tr.speed_hz = 0;  /* Use default speed */
    tr.bits_per_word = 0;  /* Use default bits */
    
    ret = ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 0) {
        printf("[ST7789] ERROR: SPI transfer failed (ret=%d, len=%d)\n", ret, len);
        perror("ioctl SPI_IOC_MESSAGE");
    }
}
