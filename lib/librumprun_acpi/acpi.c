#include <bmk-core/platform.h>
#include <bmk-core/printf.h>

#include <rumprun-acpi/acpica.h>
#include <rumprun-acpi/acpi.h>
#include <rumprun-acpi/acpi_utils.h>

static ACPI_TABLE_HEADER *
acpi_map_rsdt(void)
{
        ACPI_PHYSICAL_ADDRESS paddr;
        ACPI_TABLE_RSDP *rsdp;

        paddr = AcpiOsGetRootPointer();

        if (paddr == 0)
		return NULL;

        rsdp = AcpiOsMapMemory(paddr, sizeof(ACPI_TABLE_RSDP));

        if (rsdp == NULL)
		return NULL;

        if (rsdp->Revision > 1 && rsdp->XsdtPhysicalAddress)
		paddr = rsdp->XsdtPhysicalAddress;
        else
		paddr = rsdp->RsdtPhysicalAddress;

        AcpiOsUnmapMemory(rsdp, sizeof(ACPI_TABLE_RSDP));

        return AcpiOsMapMemory(paddr, sizeof(ACPI_TABLE_HEADER));
}

static void
acpi_unmap_rsdt(ACPI_TABLE_HEADER *rsdt)
{

        if (rsdt == NULL)
		return;

        AcpiOsUnmapMemory(rsdt, sizeof(ACPI_TABLE_HEADER));
}

static UINT32
acpi_event_power_button_sleep(void *context)
{
	(void)AcpiEnterSleepStatePrep(ACPI_STATE_S5);

	(void)AcpiDisableAllGpes();

	(void)AcpiEnterSleepState(ACPI_STATE_S5);

	bmk_platform_halt("ACPI shutdown failed; dropping the hammer...");

	/* never reached */
	return ACPI_INTERRUPT_HANDLED;
}

int bmk_acpi_init(void)
{
	ACPI_TABLE_HEADER *rsdt;
	ACPI_STATUS rv;

	AcpiGbl_EnableInterpreterSlack = 1;
	rv = AcpiInitializeSubsystem();

	if (ACPI_FAILURE(rv)) {
		bmk_printf("%s: failed to initialize subsytem\n", __func__);
		return 0;
	}

	rv = AcpiInitializeTables(NULL, 2, 1);

	if (ACPI_FAILURE(rv)) {
		bmk_printf("%s: failed to initialize tables\n", __func__);
		goto fail;
	}

	rv = AcpiLoadTables();
	if (ACPI_FAILURE(rv)) {
		bmk_printf("%s: failed to load tables\n", __func__);
		goto fail;
	}

	rsdt = acpi_map_rsdt();
	if (rsdt == NULL) {
		bmk_printf("%s: failed to map RSDT\n", __func__);
		goto fail;
	}

	bmk_printf("ACPI: X/RSDT: OemId <%6.6s,%8.8s,%08x>, "
	    "AslId <%4.4s,%08x>\n", rsdt->OemId, rsdt->OemTableId,
	    rsdt->OemRevision, rsdt->AslCompilerId,
	    rsdt->AslCompilerRevision);

	acpi_unmap_rsdt(rsdt);

	bmk_acpi_table_walk(ACPI_SIG_MADT, bmk_dump_madt_header);

	rv = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE(rv)) {
		bmk_printf("%s: failed to enable subsystem\n", __func__);
		goto fail;
	}

	rv = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE(rv)) {
		bmk_printf("%s: failed to initialize objects\n", __func__);
		goto fail;
	}

	AcpiClearEvent(ACPI_EVENT_POWER_BUTTON);
	AcpiInstallFixedEventHandler(ACPI_EVENT_POWER_BUTTON,
	    acpi_event_power_button_sleep, NULL);

	return 1;

fail:
	(void)AcpiTerminate();

	return 0;
}
