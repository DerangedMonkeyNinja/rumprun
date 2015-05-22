.include <bsd.own.mk>

RUMPTOP=	${TOPRUMP}

LIB=	rumprun_acpi

CPPFLAGS+=	\
	-I${RUMPTOP}/../dev/acpi \
	-I${.CURDIR}/../../include \
	-I${.CURDIR}/../../include/rumprun-acpi/acpica

# Disallow type-punning in the acpica code base
CFLAGS+=	-DACPI_MISALIGNMENT_NOT_SUPPORTED

.if ${MACHINE} == "i386"
CFLAGS+=	-DACPI_32BIT_PHYSICAL_ADDRESS
.endif

.PATH:	${RUMPTOP}/../external/bsd/acpica/dist

SRCS_ACPICA_DEBUG := \
	common/ahids.c \
	common/ahuuids.c \
	debugger/dbcmds.c \
	debugger/dbconvert.c \
	debugger/dbdisply.c \
	debugger/dbexec.c \
	debugger/dbfileio.c \
	debugger/dbhistry.c \
	debugger/dbinput.c \
	debugger/dbmethod.c \
	debugger/dbnames.c \
	debugger/dbstats.c \
	debugger/dbtest.c \
	debugger/dbutils.c \
	debugger/dbxface.c \
	disassembler/dmbuffer.c \
	disassembler/dmcstyle.c \
	disassembler/dmdeferred.c \
	disassembler/dmnames.c \
	disassembler/dmopcode.c \
	disassembler/dmobject.c \
	disassembler/dmresrc.c \
	disassembler/dmresrcl.c \
	disassembler/dmresrcl2.c \
	disassembler/dmresrcs.c \
	disassembler/dmutils.c \
	disassembler/dmwalk.c \
	resources/rsdumpinfo.c

SRCS_ACPICA_CORE := \
	dispatcher/dsargs.c \
	dispatcher/dscontrol.c \
	dispatcher/dsfield.c \
	dispatcher/dsinit.c \
	dispatcher/dsmethod.c \
	dispatcher/dsmthdat.c \
	dispatcher/dsobject.c \
	dispatcher/dsopcode.c \
	dispatcher/dsutils.c \
	dispatcher/dswexec.c \
	dispatcher/dswload.c \
	dispatcher/dswload2.c \
	dispatcher/dswscope.c \
	dispatcher/dswstate.c \
	events/evevent.c \
	events/evglock.c \
	events/evgpe.c \
	events/evgpeblk.c \
	events/evgpeinit.c \
	events/evgpeutil.c \
	events/evhandler.c \
	events/evmisc.c \
	events/evregion.c \
	events/evrgnini.c \
	events/evsci.c \
	events/evxface.c \
	events/evxfevnt.c \
	events/evxfgpe.c \
	events/evxfregn.c \
	executer/exconfig.c \
	executer/exconvrt.c \
	executer/excreate.c \
	executer/exdebug.c \
	executer/exdump.c \
	executer/exfield.c \
	executer/exfldio.c \
	executer/exmisc.c \
	executer/exmutex.c \
	executer/exnames.c \
	executer/exoparg1.c \
	executer/exoparg2.c \
	executer/exoparg3.c \
	executer/exoparg6.c \
	executer/exprep.c \
	executer/exregion.c \
	executer/exresnte.c \
	executer/exresolv.c \
	executer/exresop.c \
	executer/exstore.c \
	executer/exstoren.c \
	executer/exstorob.c \
	executer/exsystem.c \
	executer/exutils.c \
	hardware/hwacpi.c \
	hardware/hwesleep.c \
	hardware/hwgpe.c \
	hardware/hwpci.c \
	hardware/hwregs.c \
	hardware/hwsleep.c \
	hardware/hwtimer.c \
	hardware/hwvalid.c \
	hardware/hwxface.c \
	hardware/hwxfsleep.c \
	namespace/nsaccess.c \
	namespace/nsalloc.c \
	namespace/nsarguments.c \
	namespace/nsconvert.c \
	namespace/nsdump.c \
	namespace/nseval.c \
	namespace/nsinit.c \
	namespace/nsload.c \
	namespace/nsnames.c \
	namespace/nsobject.c \
	namespace/nsparse.c \
	namespace/nspredef.c \
	namespace/nsprepkg.c \
	namespace/nsrepair.c \
	namespace/nsrepair2.c \
	namespace/nssearch.c \
	namespace/nsutils.c \
	namespace/nswalk.c \
	namespace/nsxfeval.c \
	namespace/nsxfname.c \
	namespace/nsxfobj.c \
	parser/psargs.c \
	parser/psloop.c \
	parser/psobject.c \
	parser/psopcode.c \
	parser/psopinfo.c \
	parser/psparse.c \
	parser/psscope.c \
	parser/pstree.c \
	parser/psutils.c \
	parser/pswalk.c \
	parser/psxface.c \
	resources/rsaddr.c \
	resources/rscalc.c \
	resources/rscreate.c \
	resources/rsdump.c \
	resources/rsinfo.c \
	resources/rsio.c \
	resources/rsirq.c \
	resources/rslist.c \
	resources/rsmemory.c \
	resources/rsmisc.c \
	resources/rsserial.c \
	resources/rsutils.c \
	resources/rsxface.c \
	tables/tbdata.c \
	tables/tbfadt.c \
	tables/tbfind.c \
	tables/tbinstal.c \
	tables/tbprint.c \
	tables/tbutils.c \
	tables/tbxface.c \
	tables/tbxfload.c \
	tables/tbxfroot.c \
	utilities/utaddress.c \
	utilities/utalloc.c \
	utilities/utbuffer.c \
	utilities/utcache.c \
	utilities/utcopy.c \
	utilities/utdebug.c \
	utilities/utdecode.c \
	utilities/utdelete.c \
	utilities/uterror.c \
	utilities/uteval.c \
	utilities/utexcep.c \
	utilities/utglobal.c \
	utilities/uthex.c \
	utilities/utids.c \
	utilities/utinit.c \
	utilities/utlock.c \
	utilities/utmath.c \
	utilities/utmisc.c \
	utilities/utmutex.c \
	utilities/utobject.c \
	utilities/utosi.c \
	utilities/utownerid.c \
	utilities/utpredef.c \
	utilities/utresrc.c \
	utilities/utstate.c \
	utilities/utstring.c \
	utilities/utuuid.c \
	utilities/utxface.c \
	utilities/utxferror.c \
	utilities/utxfinit.c

SRCS_ACPICA_GLUE := \
	acpica/OsdEnvironment.c \
	acpica/OsdHardware.c \
	acpica/OsdInterrupt.c \
	acpica/OsdMemory.c \
	acpica/OsdMisc.c \
	acpica/OsdSchedule.c \
	acpica/OsdStream.c \
	acpica/OsdSynch.c

SRCS_RUMP := \
	acpi.c \
	acpi_madt.c \
	acpi_tables.c

SRCS := $(SRCS_RUMP) $(SRCS_ACPICA_CORE) $(SRCS_ACPICA_GLUE)

#OBJS := $(notdir $(SRCS))

SUBDIRS := acpica common compiler debugger disassembler dispatcher \
	events executer hardware namespace parser resources tables \
	utilities
beforedepend:
	@mkdir -p $(SUBDIRS)

cleandir:
	@rm -rf $(SUBDIRS)

.include <bsd.lib.mk>
