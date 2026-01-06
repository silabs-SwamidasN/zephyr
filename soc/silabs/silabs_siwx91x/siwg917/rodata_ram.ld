
#include <zephyr/linker/linker-defs.h>
#include <zephyr/linker/linker-tool.h>

/*
 * Section pour reloger TOUT .rodata en RAM
 * Cette section sera inject├⌐e dans snippets-rom-sections.ld
 * AVANT la section rodata principale du linker script
 *
 * La section est plac├⌐e en RAM (VMA) mais charg├⌐e depuis FLASH (LMA)
 * Les symboles __rodata_ram_load_start, __rodata_ram_start, etc.
 * sont disponibles pour copier les donn├⌐es au d├⌐marrage si n├⌐cessaire
 */
.rodata_ram :
{
	__rodata_ram_start = .;
	*(.rodata)
	*(.rodata.*)
	*(.gnu.linkonce.r.*)
	__rodata_ram_end = .;
} GROUP_DATA_LINK_IN(RAMABLE_REGION, ROMABLE_REGION)

__rodata_ram_load_start = LOADADDR(.rodata_ram);
__rodata_ram_size = __rodata_ram_end - __rodata_ram_start;

