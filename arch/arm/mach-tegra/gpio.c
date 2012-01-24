/*
 * arch/arm/mach-tegra/gpio.c
 *
 * Copyright (c) 2010 Google, Inc
 *
 * Author:
 *	Erik Gilling <konkers@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#include <linux/io.h>
#include <linux/gpio.h>

#include <mach/iomap.h>
#include <mach/suspend.h>

#define GPIO_BANK(x)		((x) >> 5)
#define GPIO_PORT(x)		(((x) >> 3) & 0x3)
#define GPIO_BIT(x)		((x) & 0x7)

#define GPIO_REG(x)		(IO_TO_VIRT(TEGRA_GPIO_BASE) +	\
				 GPIO_BANK(x) * 0x80 +		\
				 GPIO_PORT(x) * 4)

#define GPIO_CNF(x)		(GPIO_REG(x) + 0x00)
#define GPIO_OE(x)		(GPIO_REG(x) + 0x10)
#define GPIO_OUT(x)		(GPIO_REG(x) + 0X20)
#define GPIO_IN(x)		(GPIO_REG(x) + 0x30)
#define GPIO_INT_STA(x)		(GPIO_REG(x) + 0x40)
#define GPIO_INT_ENB(x)		(GPIO_REG(x) + 0x50)
#define GPIO_INT_LVL(x)		(GPIO_REG(x) + 0x60)
#define GPIO_INT_CLR(x)		(GPIO_REG(x) + 0x70)

#define GPIO_MSK_CNF(x)		(GPIO_REG(x) + 0x800)
#define GPIO_MSK_OE(x)		(GPIO_REG(x) + 0x810)
#define GPIO_MSK_OUT(x)		(GPIO_REG(x) + 0X820)
#define GPIO_MSK_INT_STA(x)	(GPIO_REG(x) + 0x840)
#define GPIO_MSK_INT_ENB(x)	(GPIO_REG(x) + 0x850)
#define GPIO_MSK_INT_LVL(x)	(GPIO_REG(x) + 0x860)

#define GPIO_INT_LVL_MASK		0x010101
#define GPIO_INT_LVL_EDGE_RISING	0x000101
#define GPIO_INT_LVL_EDGE_FALLING	0x000100
#define GPIO_INT_LVL_EDGE_BOTH		0x010100
#define GPIO_INT_LVL_LEVEL_HIGH		0x000001
#define GPIO_INT_LVL_LEVEL_LOW		0x000000

struct tegra_gpio_bank {
	int bank;
	int irq;
	spinlock_t lvl_lock[4];
#ifdef CONFIG_PM
	u32 cnf[4];
	u32 out[4];
	u32 oe[4];
	u32 int_enb[4];
	u32 int_lvl[4];
#endif
};


static struct tegra_gpio_bank tegra_gpio_banks[] = {
	{.bank = 0, .irq = INT_GPIO1},
	{.bank = 1, .irq = INT_GPIO2},
	{.bank = 2, .irq = INT_GPIO3},
	{.bank = 3, .irq = INT_GPIO4},
	{.bank = 4, .irq = INT_GPIO5},
	{.bank = 5, .irq = INT_GPIO6},
	{.bank = 6, .irq = INT_GPIO7},
};

static int tegra_gpio_compose(int bank, int port, int bit)
{
	return (bank << 5) | ((port & 0x3) << 3) | (bit & 0x7);
}

static void tegra_gpio_mask_write(u32 reg, int gpio, int value)
{
	u32 val;

	val = 0x100 << GPIO_BIT(gpio);
	if (value)
		val |= 1 << GPIO_BIT(gpio);
	__raw_writel(val, reg);
}

void tegra_gpio_enable(int gpio)
{
	tegra_gpio_mask_write(GPIO_MSK_CNF(gpio), gpio, 1);
}

void tegra_gpio_disable(int gpio)
{
	tegra_gpio_mask_write(GPIO_MSK_CNF(gpio), gpio, 0);
}

static void tegra_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	tegra_gpio_mask_write(GPIO_MSK_OUT(offset), offset, value);
}

static int tegra_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	return (__raw_readl(GPIO_IN(offset)) >> GPIO_BIT(offset)) & 0x1;
}

static int tegra_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	tegra_gpio_mask_write(GPIO_MSK_OE(offset), offset, 0);
	return 0;
}

static int tegra_gpio_direction_output(struct gpio_chip *chip, unsigned offset,
					int value)
{
	tegra_gpio_set(chip, offset, value);
	tegra_gpio_mask_write(GPIO_MSK_OE(offset), offset, 1);
	return 0;
}



static struct gpio_chip tegra_gpio_chip = {
	.label			= "tegra-gpio",
	.direction_input	= tegra_gpio_direction_input,
	.get			= tegra_gpio_get,
	.direction_output	= tegra_gpio_direction_output,
	.set			= tegra_gpio_set,
	.base			= 0,
	.ngpio			= TEGRA_NR_GPIOS,
};

static void tegra_gpio_irq_ack(unsigned int irq)
{
	int gpio = irq - INT_GPIO_BASE;

	__raw_writel(1 << GPIO_BIT(gpio), GPIO_INT_CLR(gpio));
}

static void tegra_gpio_irq_mask(unsigned int irq)
{
	int gpio = irq - INT_GPIO_BASE;

	tegra_gpio_mask_write(GPIO_MSK_INT_ENB(gpio), gpio, 0);
}

static void tegra_gpio_irq_unmask(unsigned int irq)
{
	int gpio = irq - INT_GPIO_BASE;

	tegra_gpio_mask_write(GPIO_MSK_INT_ENB(gpio), gpio, 1);
}

static int tegra_gpio_irq_set_type(unsigned int irq, unsigned int type)
{
	int gpio = irq - INT_GPIO_BASE;
	struct tegra_gpio_bank *bank = get_irq_chip_data(irq);
	int port = GPIO_PORT(gpio);
	int lvl_type;
	int val;
	unsigned long flags;

	switch (type & IRQ_TYPE_SENSE_MASK) {
	case IRQ_TYPE_EDGE_RISING:
		lvl_type = GPIO_INT_LVL_EDGE_RISING;
		break;

	case IRQ_TYPE_EDGE_FALLING:
		lvl_type = GPIO_INT_LVL_EDGE_FALLING;
		break;

	case IRQ_TYPE_EDGE_BOTH:
		lvl_type = GPIO_INT_LVL_EDGE_BOTH;
		break;

	case IRQ_TYPE_LEVEL_HIGH:
		lvl_type = GPIO_INT_LVL_LEVEL_HIGH;
		break;

	case IRQ_TYPE_LEVEL_LOW:
		lvl_type = GPIO_INT_LVL_LEVEL_LOW;
		break;

	default:
		return -EINVAL;
	}

	spin_lock_irqsave(&bank->lvl_lock[port], flags);

	val = __raw_readl(GPIO_INT_LVL(gpio));
	val &= ~(GPIO_INT_LVL_MASK << GPIO_BIT(gpio));
	val |= lvl_type << GPIO_BIT(gpio);
	__raw_writel(val, GPIO_INT_LVL(gpio));

	spin_unlock_irqrestore(&bank->lvl_lock[port], flags);

	if (type & (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH))
		__set_irq_handler_unlocked(irq, handle_level_irq);
	else if (type & (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING))
		__set_irq_handler_unlocked(irq, handle_edge_irq);

	tegra_set_lp0_wake_type(irq, type);

	return 0;
}

static void tegra_gpio_irq_handler(unsigned int irq, struct irq_desc *desc)
{
	struct tegra_gpio_bank *bank;
	int port;
	int pin;
	int unmasked = 0;

	desc->chip->ack(irq);

	bank = get_irq_data(irq);

	for (port = 0; port < 4; port++) {
		int gpio = tegra_gpio_compose(bank->bank, port, 0);
		unsigned long sta = __raw_readl(GPIO_INT_STA(gpio)) &
			__raw_readl(GPIO_INT_ENB(gpio));
		u32 lvl = __raw_readl(GPIO_INT_LVL(gpio));

		for_each_set_bit(pin, &sta, 8) {
			__raw_writel(1 << pin, GPIO_INT_CLR(gpio));

			/* if gpio is edge triggered, clear condition
			 * before executing the hander so that we don't
			 * miss edges
			 */
			if (lvl & (0x100 << pin)) {
				unmasked = 1;
				desc->chip->unmask(irq);
			}

			generic_handle_irq(gpio_to_irq(gpio + pin));
		}
	}

	if (!unmasked)
		desc->chip->unmask(irq);

}

#ifdef CONFIG_PM

#ifdef CONFIG_MACH_SAMSUNG_VARIATION_TEGRA
#include <mach/gpio-sec.h>

struct sec_gpio_table_st sec_gpio_table;

void tegra_set_sleep_gpio_table(void)
{
	int cnt;
	struct sec_slp_gpio_cfg_st *sleep_gpio_table = sec_gpio_table.sleep_gpio_table;
	int n_sleep_gpio_table = sec_gpio_table.n_sleep_gpio_table;

	if(!sleep_gpio_table) {
		pr_err("%s: gpio_table is null\n", __func__);
		return;
	}
	
	for (cnt = 0; cnt < n_sleep_gpio_table; cnt++) {
		if (sleep_gpio_table[cnt].slp_ctrl == YES) {
			tegra_gpio_enable(sleep_gpio_table[cnt].gpio);

			if (sleep_gpio_table[cnt].dir == GPIO_OUTPUT) {
				tegra_gpio_mask_write(GPIO_MSK_OE(sleep_gpio_table[cnt].gpio), sleep_gpio_table[cnt].gpio, 1);
				if (sleep_gpio_table[cnt].val != GPIO_LEVEL_NONE) {
					tegra_gpio_mask_write(GPIO_MSK_OUT(sleep_gpio_table[cnt].gpio),
									sleep_gpio_table[cnt].gpio, sleep_gpio_table[cnt].val);
				}
			} else if (sleep_gpio_table[cnt].dir == GPIO_INPUT) {
				tegra_gpio_mask_write(GPIO_MSK_OE(sleep_gpio_table[cnt].gpio), sleep_gpio_table[cnt].gpio, 0);
			}
		}
	}
}

void tegra_set_gpio_init_table(void)
{
	int cnt;
	struct sec_gpio_cfg_st *gpio_table = sec_gpio_table.init_gpio_table;
	int n_gpio_table = sec_gpio_table.n_init_gpio_table;

	if(!gpio_table) {
		pr_err("%s: gpio_table is null\n", __func__);
		return;
	}
	
	for (cnt = 0; cnt < n_gpio_table; cnt++) {
		if (gpio_table[cnt].attr == GPIO) {
			tegra_gpio_enable(gpio_table[cnt].gpio);

			if (gpio_table[cnt].dir == GPIO_OUTPUT) {
				tegra_gpio_mask_write(GPIO_MSK_OE(gpio_table[cnt].gpio), gpio_table[cnt].gpio, 1);
				if (gpio_table[cnt].val != GPIO_LEVEL_NONE) {
					tegra_gpio_mask_write(GPIO_MSK_OUT(gpio_table[cnt].gpio), gpio_table[cnt].gpio, gpio_table[cnt].val);
				}
			} else if (gpio_table[cnt].dir == GPIO_INPUT) {
				tegra_gpio_mask_write(GPIO_MSK_OE(gpio_table[cnt].gpio), gpio_table[cnt].gpio, 0);
			} else {
				tegra_gpio_disable(gpio_table[cnt].gpio);
			}
		} else {
			tegra_gpio_disable(gpio_table[cnt].gpio);
		}
	}
}
#endif /* CONFIG_MACH_SAMSUNG_VARIATION_TEGRA */

void tegra_gpio_resume(void)
{
	unsigned long flags;
	int b, p, i;

	local_irq_save(flags);

	for (b = 0; b < ARRAY_SIZE(tegra_gpio_banks); b++) {
		struct tegra_gpio_bank *bank = &tegra_gpio_banks[b];

		for (p = 0; p < ARRAY_SIZE(bank->oe); p++) {
			unsigned int gpio = (b<<5) | (p<<3);
			__raw_writel(bank->cnf[p], GPIO_CNF(gpio));
			__raw_writel(bank->out[p], GPIO_OUT(gpio));
			__raw_writel(bank->oe[p], GPIO_OE(gpio));
			__raw_writel(bank->int_lvl[p], GPIO_INT_LVL(gpio));
			__raw_writel(bank->int_enb[p], GPIO_INT_ENB(gpio));
		}
	}

	local_irq_restore(flags);

	for (i = INT_GPIO_BASE; i < (INT_GPIO_BASE + TEGRA_NR_GPIOS); i++) {
		struct irq_desc *desc = irq_to_desc(i);
		if (!desc || (desc->status & IRQ_WAKEUP))
			continue;
		enable_irq(i);
	}
}

void tegra_gpio_suspend(void)
{
	unsigned long flags;
	int b, p, i;

	for (i = INT_GPIO_BASE; i < (INT_GPIO_BASE + TEGRA_NR_GPIOS); i++) {
		struct irq_desc *desc = irq_to_desc(i);
		if (!desc)
			continue;
		if (desc->status & IRQ_WAKEUP) {
			int gpio = i - INT_GPIO_BASE;
			pr_debug("gpio %d.%d is wakeup\n", gpio/8, gpio&7);
			continue;
		}
		disable_irq(i);
	}

	local_irq_save(flags);
	for (b = 0; b < ARRAY_SIZE(tegra_gpio_banks); b++) {
		struct tegra_gpio_bank *bank = &tegra_gpio_banks[b];

		for (p = 0; p < ARRAY_SIZE(bank->oe); p++) {
			unsigned int gpio = (b<<5) | (p<<3);
			bank->cnf[p] = __raw_readl(GPIO_CNF(gpio));
			bank->out[p] = __raw_readl(GPIO_OUT(gpio));
			bank->oe[p] = __raw_readl(GPIO_OE(gpio));
			bank->int_enb[p] = __raw_readl(GPIO_INT_ENB(gpio));
			bank->int_lvl[p] = __raw_readl(GPIO_INT_LVL(gpio));
		}
	}
	local_irq_restore(flags);

#ifdef CONFIG_MACH_SAMSUNG_VARIATION_TEGRA
	tegra_set_sleep_gpio_table();
#endif
}

static int tegra_gpio_wake_enable(unsigned int irq, unsigned int enable)
{
	int ret;
	struct tegra_gpio_bank *bank = get_irq_chip_data(irq);

	ret = tegra_set_lp1_wake(bank->irq, enable);
	ret |= tegra_set_lp0_wake(irq, enable);
	if (ret)
		return ret;

	return 0;
}
#endif

static struct irq_chip tegra_gpio_irq_chip = {
	.name		= "GPIO",
	.ack		= tegra_gpio_irq_ack,
	.mask		= tegra_gpio_irq_mask,
	.unmask		= tegra_gpio_irq_unmask,
	.set_type	= tegra_gpio_irq_set_type,
#ifdef CONFIG_PM
	.set_wake	= tegra_gpio_wake_enable,
#endif
};


/* This lock class tells lockdep that GPIO irqs are in a different
 * category than their parents, so it won't report false recursion.
 */
static struct lock_class_key gpio_lock_class;

static int __init tegra_gpio_init(void)
{
	struct tegra_gpio_bank *bank;
	int i;
	int j;

#ifdef CONFIG_MACH_SAMSUNG_VARIATION_TEGRA
	/* init the gpio when kernel booting. */
	pr_info("%s()\n", __func__);
	tegra_gpio_register_table(&sec_gpio_table);
	tegra_set_gpio_init_table();
#endif

	for (i = 0; i < 7; i++) {
		for (j = 0; j < 4; j++) {
			int gpio = tegra_gpio_compose(i, j, 0);
			__raw_writel(0x00, GPIO_INT_ENB(gpio));
			__raw_writel(0x00, GPIO_INT_STA(gpio));
		}
	}

	gpiochip_add(&tegra_gpio_chip);

	for (i = INT_GPIO_BASE; i < (INT_GPIO_BASE + TEGRA_NR_GPIOS); i++) {
		bank = &tegra_gpio_banks[GPIO_BANK(irq_to_gpio(i))];

		lockdep_set_class(&irq_desc[i].lock, &gpio_lock_class);
		set_irq_chip_data(i, bank);
		set_irq_chip(i, &tegra_gpio_irq_chip);
		set_irq_handler(i, handle_simple_irq);
		set_irq_flags(i, IRQF_VALID);
	}

	for (i = 0; i < ARRAY_SIZE(tegra_gpio_banks); i++) {
		bank = &tegra_gpio_banks[i];

		for (j = 0; j < 4; j++)
			spin_lock_init(&bank->lvl_lock[j]);

		set_irq_data(bank->irq, bank);
		set_irq_chained_handler(bank->irq, tegra_gpio_irq_handler);

	}

	return 0;
}

postcore_initcall(tegra_gpio_init);

#ifdef	CONFIG_DEBUG_FS

#include <linux/debugfs.h>
#include <linux/seq_file.h>

static int dbg_gpio_show(struct seq_file *s, void *unused)
{
	int i;
	int j;

	for (i = 0; i < 7; i++) {
		for (j = 0; j < 4; j++) {
			int gpio = tegra_gpio_compose(i, j, 0);
			seq_printf(s,
				"%d:%d %02x %02x %02x %02x %02x %02x %06x\n",
				i, j,
				__raw_readl(GPIO_CNF(gpio)),
				__raw_readl(GPIO_OE(gpio)),
				__raw_readl(GPIO_OUT(gpio)),
				__raw_readl(GPIO_IN(gpio)),
				__raw_readl(GPIO_INT_STA(gpio)),
				__raw_readl(GPIO_INT_ENB(gpio)),
				__raw_readl(GPIO_INT_LVL(gpio)));
		}
	}
	return 0;
}

static int dbg_gpio_open(struct inode *inode, struct file *file)
{
	return single_open(file, dbg_gpio_show, &inode->i_private);
}

static const struct file_operations debug_fops = {
	.open		= dbg_gpio_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init tegra_gpio_debuginit(void)
{
	(void) debugfs_create_file("tegra_gpio", S_IRUGO,
					NULL, NULL, &debug_fops);
	return 0;
}
late_initcall(tegra_gpio_debuginit);
#endif
