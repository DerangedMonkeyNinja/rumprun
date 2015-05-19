#ifndef _RUMP_ACPI_MACHDEP_H_
#define _RUMP_ACPI_MACHDEP_H_

UINT8  acpi_md_OsIn8(UINT16);
UINT16 acpi_md_OsIn16(UINT16);
UINT32 acpi_md_OsIn32(UINT16);

void acpi_md_OsOut8(UINT16, UINT8);
void acpi_md_OsOut16(UINT16, UINT16);
void acpi_md_OsOut32(UINT16, UINT32);

ACPI_STATUS acpi_md_OsInstallInterruptHandler(UINT32,
                                              ACPI_OSD_HANDLER,
                                              void *, void **);
void        acpi_md_OsRemoveInterruptHandler(void *);

ACPI_STATUS acpi_md_OsMapMemory(ACPI_PHYSICAL_ADDRESS, UINT32, void **);
void        acpi_md_OsUnmapMemory(void *, UINT32);
ACPI_STATUS acpi_md_OsGetPhysicalAddress(void *LogicalAddress,
                                         ACPI_PHYSICAL_ADDRESS *PhysicalAddress);

BOOLEAN     acpi_md_OsReadable(void *, UINT32);
BOOLEAN     acpi_md_OsWritable(void *, UINT32);

#endif
