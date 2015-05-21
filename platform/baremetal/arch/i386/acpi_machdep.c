#include <bmk/kernel.h>
#include <bmk/types.h>
#include <bmk/machine/md.h>

uint8_t  acpi_md_OsIn8(uint16_t port);
uint16_t acpi_md_OsIn16(uint16_t port);
uint32_t acpi_md_OsIn32(uint16_t port);

void acpi_md_OsOut8(uint16_t port, uint8_t value);
void acpi_md_OsOut16(uint16_t port, uint16_t value);
void acpi_md_OsOut32(uint16_t port, uint32_t value);

int acpi_md_OsInstallInterruptHandler(uint32_t, uint32_t (*fn)(void *), void *, void**);
void acpi_md_OsRemoveInterruptHandler(void *);

int acpi_md_OsMapMemory(uintptr_t, uint32_t, void **);
void acpi_md_OsUnmapMemory(void *LogicalAddress, uint32_t Length);
int acpi_md_OsGetPhysicalAddress(void *LogicalAddress,
    uintptr_t *PhysicalAddress);

int acpi_md_OsReadable(void *Pointer, uint32_t Length);
int acpi_md_OsWritable(void *Pointer, uint32_t Length);

uint8_t acpi_md_OsIn8(uint16_t port)
{
	return inb(port);
}

uint16_t acpi_md_OsIn16(uint16_t port)
{
        uint16_t rv;

        __asm__ __volatile__("inw %1, %0" : "=a"(rv) : "d"(port));

        return rv;
}

uint32_t acpi_md_OsIn32(uint16_t port)
{
	return inl(port);
}

void acpi_md_OsOut8(uint16_t port, uint8_t value)
{
	outb(port, value);
}
void acpi_md_OsOut16(uint16_t port, uint16_t value)
{
        __asm__ __volatile__("outw %0, %1" :: "a"(value), "d"(port));
}

void acpi_md_OsOut32(uint16_t port, uint32_t value)
{
	outl(port, value);
}

int acpi_md_OsInstallInterruptHandler(uint32_t InterruptNumber, uint32_t (*Handler)(void *), void *Context, void **aih_ih)
{
	(void)aih_ih;
	return bmk_isr_init((int (*)(void *))Handler, Context, InterruptNumber);
}

void acpi_md_OsRemoveInterruptHandler(void *aih_ih)
{
	/* XXX: not yet */
}

int acpi_md_OsMapMemory(uintptr_t PhysicalAddress, uint32_t Length, void **LogicalAddress)
{
	*LogicalAddress = (void *)PhysicalAddress;
	return 0;
}

void acpi_md_OsUnmapMemory(void *LogicalAddress, uint32_t Length)
{

}

int acpi_md_OsGetPhysicalAddress(void *LogicalAddress,
    uintptr_t *PhysicalAddress)
{
	*PhysicalAddress = (uintptr_t)LogicalAddress;
	return 0;
}

int acpi_md_OsReadable(void *Pointer, uint32_t Length)
{
	return 1;
}

int acpi_md_OsWritable(void *Pointer, uint32_t Length)
{
	return 1;
}
