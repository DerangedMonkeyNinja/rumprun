#ifndef _RUMP_ACPI_H_
#define _RUMP_ACPI_H_

#include <rumprun-acpi/acpica.h>

int bmk_acpi_init(void);

ACPI_STATUS bmk_acpi_table_walk(const char *table_sig,
    ACPI_STATUS (*callback)(ACPI_SUBTABLE_HEADER *));

#endif /* _RUMP_ACPI_H_ */
