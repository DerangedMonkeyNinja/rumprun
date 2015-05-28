#include <bmk-core/platform.h>
#include <bmk-core/printf.h>

#include <rumprun-acpi/acpica.h>
#include <rumprun-acpi/acpi.h>
#include <rumprun-acpi/acpi_utils.h>

#include "i82489reg.h"
#include "ioapic.h"
#include "lapic.h"
#include "mpacpi.h"

int mpacpi_ncpus = 0;
int mpacpi_nioapics = 0;
int mpacpi_nintsrc = 0;
uintptr_t lapic_base = LAPIC_BASE;

static ACPI_STATUS
mpacpi_madt_enumerate(ACPI_SUBTABLE_HEADER *header)
{
	ACPI_MADT_LOCAL_APIC_OVERRIDE *lao = NULL;

	switch (header->Type) {
	case ACPI_MADT_TYPE_LOCAL_APIC:
		mpacpi_ncpus++;
		break;
	case ACPI_MADT_TYPE_IO_APIC:
		mpacpi_nioapics++;
		break;
	case ACPI_MADT_TYPE_NMI_SOURCE:
	case ACPI_MADT_TYPE_LOCAL_APIC_NMI:
		mpacpi_nintsrc++;
		break;
	case ACPI_MADT_TYPE_LOCAL_APIC_OVERRIDE:
		lao = (ACPI_MADT_LOCAL_APIC_OVERRIDE *)header;
		lapic_base = lao->Address;
		break;
	default:
		bmk_printf("%s: No handler for type: 0x%x\n", __func__, header->Type);
		break;
	}

	return AE_OK;
}

static ACPI_STATUS
mpacpi_enable_ioapic(ACPI_SUBTABLE_HEADER *header) {
	ACPI_MADT_IO_APIC *ioapic = NULL;

	if (header->Type == ACPI_MADT_TYPE_IO_APIC) {
		ioapic = (ACPI_MADT_IO_APIC *)header;
		ioapic_enable(ioapic->Address);
	}

	return AE_OK;
}

static ACPI_STATUS
mpacpi_configure_ioapic(ACPI_SUBTABLE_HEADER *header) {

	ACPI_MADT_INTERRUPT_OVERRIDE *irq_ovr = NULL;

	switch (header->Type) {
	case ACPI_MADT_TYPE_INTERRUPT_OVERRIDE:
		irq_ovr = (ACPI_MADT_INTERRUPT_OVERRIDE *)header;
		ioapic_init_override(irq_ovr->SourceIrq, irq_ovr->GlobalIrq, irq_ovr->IntiFlags);
		break;
	default:
		bmk_printf("%s: no handler for type 0x%x\n", __func__, header->Type);
		break;
	}

	return AE_OK;
}


/*
 * Set the PIC mode
 * PIC = 0, APIC = 1, SAPIC = 2
 */
static ACPI_STATUS
mpacpi_set_pic_mode(int mode)
{
	ACPI_OBJECT object = {
		.Type = ACPI_TYPE_INTEGER,
		.Integer.Value = mode,
	};

	ACPI_OBJECT_LIST items = {
		.Count = 1,
		.Pointer = &object,
	};

	return AcpiEvaluateObject(ACPI_ROOT_OBJECT, "\\_PIC", &items, NULL);
}

void
mpacpi_init(void)
{
	ACPI_STATUS rv;

	bmk_acpi_table_walk(ACPI_SIG_MADT, mpacpi_madt_enumerate);
	if (mpacpi_ncpus <= 1)
		return;

	/* Parse the MADT table and setup interrupts */
	if (mpacpi_nioapics != 1)
		bmk_platform_halt("too many ioapics");

	/* Commit to APIC mode */
	rv = mpacpi_set_pic_mode(1);
	if (ACPI_FAILURE(rv) && rv != AE_NOT_FOUND) {
		bmk_printf("%s: switch to APIC mode failed\n", __func__);
		return;
	}

	/*
	 * The subtables under MADT aren't necessarily ordered, so
	 * we enable the ioapic before we parse the MADT for configuration
	 * data.
	 */
	bmk_acpi_table_walk(ACPI_SIG_MADT, mpacpi_enable_ioapic);
	bmk_acpi_table_walk(ACPI_SIG_MADT, mpacpi_configure_ioapic);

	/* Setup the Local APIC on the BSP */
	lapic_boot_init(lapic_base);
	lapic_enable();
	lapic_set_lvt();
}
