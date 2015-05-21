#include <string.h>

#include <bmk-core/platform.h>

#include <rumprun-acpi/acpi.h>

static
size_t acpi_get_table_size(const char *sig)
{
	size_t size = 0;

	if (strcmp(ACPI_SIG_MADT, sig) == 0)
		size = sizeof(ACPI_TABLE_MADT);
	else
		bmk_platform_halt("Fix me!");

	return (size);
}

ACPI_STATUS
bmk_acpi_table_walk(const char *table_sig, ACPI_STATUS (*callback)(ACPI_SUBTABLE_HEADER *))
{
	ACPI_TABLE_HEADER *table;
	ACPI_SUBTABLE_HEADER *sub_hdr;
	ACPI_STATUS rv;
	char *table_end = NULL,
	     *pos = NULL;

	rv = AcpiGetTable(table_sig, 1, &table);
	if (ACPI_FAILURE(rv))
		return rv;

	table_end = (char *)table + table->Length;
	pos = (char *)table + acpi_get_table_size(table_sig);

	while (pos < table_end) {
		sub_hdr = (ACPI_SUBTABLE_HEADER *)pos;

		if (ACPI_FAILURE(rv = callback(sub_hdr)))
			return (rv);

		pos += sub_hdr->Length;
	}

	return AE_OK;
}
