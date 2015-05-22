#include <bmk-core/printf.h>
#include <rumprun-acpi/acpi.h>
#include <rumprun-acpi/acpi_utils.h>

static void dump_local_apic(ACPI_SUBTABLE_HEADER *header)
{
	ACPI_MADT_LOCAL_APIC *lapic = (ACPI_MADT_LOCAL_APIC *)header;

	bmk_printf("Local APIC: Processor ID 0x%x, APIC ID 0x%x, Flags 0x%x\n",
	    lapic->ProcessorId, lapic->Id, lapic->LapicFlags);
}

static void dump_io_apic(ACPI_SUBTABLE_HEADER *header)
{
	ACPI_MADT_IO_APIC *ioapic = (ACPI_MADT_IO_APIC *)header;

	bmk_printf("IO APIC: ID 0x%x, Address 0x%x, IRQ Base 0x%x\n",
	    ioapic->Id, ioapic->Address, ioapic->GlobalIrqBase);
}

static void dump_irq_override(ACPI_SUBTABLE_HEADER *header)
{
	ACPI_MADT_INTERRUPT_OVERRIDE *irq_override =
	    (ACPI_MADT_INTERRUPT_OVERRIDE *)header;

	bmk_printf("IRQ Override: Bus 0x%x, Src IRQ 0x%x, Global IRQ 0x%x, Flag 0x%x\n",
	    irq_override->Bus, irq_override->SourceIrq, irq_override->GlobalIrq,
	    irq_override->IntiFlags);
}

static void dump_nmi_source(ACPI_SUBTABLE_HEADER *header)
{
	ACPI_MADT_NMI_SOURCE *nmi = (ACPI_MADT_NMI_SOURCE *)header;

	bmk_printf("NMI Override: Flags 0x0%x, IRQ 0x0%x\n",
	    nmi->IntiFlags, nmi->GlobalIrq);
}

static void dump_local_apic_nmi(ACPI_SUBTABLE_HEADER *header)
{
	ACPI_MADT_LOCAL_APIC_NMI *lapic_nmi =
	    (ACPI_MADT_LOCAL_APIC_NMI *)header;

	bmk_printf("LAPIC NMI: Processor Id 0x%x, Flags 0x%x, LINT%d\n",
	    lapic_nmi->ProcessorId, lapic_nmi->IntiFlags, lapic_nmi->Lint);
}

ACPI_STATUS
bmk_dump_madt_header(ACPI_SUBTABLE_HEADER *header)
{
	switch (header->Type) {
	case ACPI_MADT_TYPE_LOCAL_APIC:
		dump_local_apic(header);
		break;
	case ACPI_MADT_TYPE_IO_APIC:
		dump_io_apic(header);
		break;
	case ACPI_MADT_TYPE_INTERRUPT_OVERRIDE:
		dump_irq_override(header);
		break;
	case ACPI_MADT_TYPE_NMI_SOURCE:
		dump_nmi_source(header);
		break;
	case ACPI_MADT_TYPE_LOCAL_APIC_NMI:
		dump_local_apic_nmi(header);
		break;
	default:
		bmk_printf("No handler for type: 0x%x\n", header->Type);
		break;
	}

	return AE_OK;
}
