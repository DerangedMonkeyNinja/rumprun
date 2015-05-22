#ifndef _RUMP_ACPI_UTILS_H_
#define _RUMP_ACPI_UTILS_H_

#include <rumprun-acpi/acpica.h>

ACPI_STATUS bmk_acpi_table_walk(const char *table_sig,
    ACPI_STATUS (*callback)(ACPI_SUBTABLE_HEADER *));

ACPI_STATUS
bmk_dump_madt_header(ACPI_SUBTABLE_HEADER *header);

#endif /* _RUMP_ACPI_UTILS_H_ */
