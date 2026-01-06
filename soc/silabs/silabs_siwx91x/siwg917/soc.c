/*
 * Copyright (c) 2023 Antmicro
 * Copyright (c) 2024 Silicon Laboratories Inc.
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdint.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sw_isr_table.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/arch/common/init.h>

#include "em_device.h"
#include "sli_siwx917_soc.h"
#include "sl_si91x_power_manager.h"
#include "sl_si91x_hal_soc_soft_reset.h"

extern char __rodata_ram_start[];
extern char __rodata_ram_end[];
extern char __rodata_ram_load_start[];

/**
 * @brief Copy .rodata from flash to RAM before any initialization
 *
 * This hook is called very early in the boot sequence, before arch_data_copy()
 * and before soc_early_init_hook(). This ensures that .rodata is available
 * in RAM before any SOC initialization code that might use it.
 */
void soc_prep_hook(void)
{
	size_t rodata_size = __rodata_ram_end - __rodata_ram_start;
	if (rodata_size > 0) {
		arch_early_memcpy(__rodata_ram_start, __rodata_ram_load_start, rodata_size);
	}
}

void soc_early_init_hook(void)
{
	__maybe_unused sl_power_peripheral_t peripheral_config = {};
	__maybe_unused sl_power_ram_retention_config_t ram_configuration = {
		.configure_ram_banks = false,
		.m4ss_ram_size_kb = (DT_REG_SIZE(DT_NODELABEL(sram0)) / 1024),
		.ulpss_ram_size_kb = 4,
	};

	SystemInit();
	if (IS_ENABLED(CONFIG_SOC_SIWX91X_PM_BACKEND_PMGR)) {
		sli_si91x_platform_init();
		sl_si91x_power_manager_init();
		sl_si91x_power_manager_remove_peripheral_requirement(&peripheral_config);
		sl_si91x_power_manager_configure_ram_retention(&ram_configuration);
		sl_si91x_power_manager_add_ps_requirement(SL_SI91X_POWER_MANAGER_PS4);
		sl_si91x_power_manager_set_clock_scaling(SL_SI91X_POWER_MANAGER_PERFORMANCE);
	}
}

void sys_arch_reboot(int type)
{
	ARG_UNUSED(type);

	sl_si91x_soc_nvic_reset();
}

/* SiWx917's bootloader requires IRQn 32 to hold payload's entry point address. */
extern void z_arm_reset(void);
Z_ISR_DECLARE_DIRECT(32, 0, z_arm_reset);
