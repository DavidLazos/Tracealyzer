#include <reg_gio.h>
#include <gio.h>
#include "bitbang.h"
#include "interface.h"

#define INP_GPIO(g) do { *(pio_base+((g)/8)) &= ~(1<<((g)%8)); } while (0)
#define SET_MODE_GPIO(g, m) do { /* clear the mode bits first, then set as necessary */ \
		INP_GPIO(g);						\
		*(pio_base+((g)/8)) |=  ((m)<<((g)%8)); } while (0)
#define OUT_GPIO(g) SET_MODE_GPIO(g, 1)

#define GPIO_SET (*(pio_base+3))  	/* sets bits which are 1, ignores bits which are 0 */
#define GPIO_CLR (*(pio_base+4)) 	/* clears bits which are 1, ignores bits which are 0 */
#define GPIO_LEV (*(pio_base+1))

static volatile uint32_t *pio_base; /* For TMS570 must be -> gioPORTA */
struct bitbang_interface *bitbang_interface;

static int tms570gpio_read(void);
static void tms570gpio_write(int tck, int tms, int tdi);
static void tms570gpio_reset(int trst, int srst);

static int tms570gpio_init(void);
static int tms570gpio_quit(void);
static int tms570gpio_speed(int);

static int is_gpio_valid(int);

static struct bitbang_interface tms570gpio_bitbang = {
	.read = tms570gpio_read,
	.write = tms570gpio_write,
	.reset = tms570gpio_reset,
};

/* GPIO numbers for each signal. Negative values are invalid */
static int tck_gpio = 0;
static int tck_gpio_mode;
static int tms_gpio = 1;
static int tms_gpio_mode;
static int tdi_gpio = 2;
static int tdi_gpio_mode;
static int tdo_gpio = 3;
static int tdo_gpio_mode;
static int trst_gpio = 4;
static int trst_gpio_mode;
static int srst_gpio = 5;
static int srst_gpio_mode;

/* Transition delay coefficients */
static unsigned int jtag_delay;

static int tms570gpio_read(void)
{
	return !!(GPIO_LEV & 1<<tdo_gpio);
}

static void tms570gpio_write(int tck, int tms, int tdi)
{
	int i;

	uint32_t set = tck<<tck_gpio | tms<<tms_gpio | tdi<<tdi_gpio;
	uint32_t clear = !tck<<tck_gpio | !tms<<tms_gpio | !tdi<<tdi_gpio;

	GPIO_SET = set;
	GPIO_CLR = clear;

	for (i = 0; i < jtag_delay; i++)
		asm volatile ("");
}

/* (1) assert or (0) deassert reset lines */
static void tms570gpio_reset(int trst, int srst)
{
	uint32_t set = 0;
	uint32_t clear = 0;

	if (trst_gpio > 0) {
		set |= !trst<<trst_gpio;
		clear |= trst<<trst_gpio;
	}

	if (srst_gpio > 0) {
		set |= !srst<<srst_gpio;
		clear |= srst<<srst_gpio;
	}

	GPIO_SET = set;
	GPIO_CLR = clear;
}

struct jtag_interface tms570gpio_interface = {
	.speed = &tms570gpio_speed,
	.init = &tms570gpio_init,
	.quit = &tms570gpio_quit,
};

static int tms570gpio_speed(int speed)
{
	jtag_delay = speed;
	return 1;
}

static int tms570gpio_init(void)
{
	gioInit();
	bitbang_interface = &tms570gpio_bitbang;

	if (!is_gpio_valid(tdo_gpio) || !is_gpio_valid(tdi_gpio) ||
		!is_gpio_valid(tck_gpio) || !is_gpio_valid(tms_gpio) ||
		(trst_gpio != -1 && !is_gpio_valid(trst_gpio)) ||
		(srst_gpio != -1 && !is_gpio_valid(srst_gpio)))
		return 0;


	pio_base = (uint32_t *)gioPORTA;

	/*
	 * Configure TDO as an input, and TDI, TCK, TMS, TRST, SRST
	 * as outputs.  Drive TDI and TCK low, and TMS/TRST/SRST high.
	 */

	INP_GPIO(tdo_gpio);

	GPIO_CLR = 1<<tdi_gpio | 1<<tck_gpio;
	GPIO_SET = 1<<tms_gpio;

	OUT_GPIO(tdi_gpio);
	OUT_GPIO(tck_gpio);
	OUT_GPIO(tms_gpio);
	if (trst_gpio != -1) {
		GPIO_SET = 1 << trst_gpio;
		OUT_GPIO(trst_gpio);
	}
	if (srst_gpio != -1) {
		GPIO_SET = 1 << srst_gpio;
		OUT_GPIO(srst_gpio);
	}

	return 1;
}

static int tms570gpio_quit(void)
{
	SET_MODE_GPIO(tdo_gpio, tdo_gpio_mode);
	SET_MODE_GPIO(tdi_gpio, tdi_gpio_mode);
	SET_MODE_GPIO(tck_gpio, tck_gpio_mode);
	SET_MODE_GPIO(tms_gpio, tms_gpio_mode);
	if (trst_gpio != -1)
		SET_MODE_GPIO(trst_gpio, trst_gpio_mode);
	if (srst_gpio != -1)
		SET_MODE_GPIO(srst_gpio, srst_gpio_mode);

	return 1;
}

static int is_gpio_valid(int gpio)
{
	return gpio >= 0 && gpio <= 7;
}

