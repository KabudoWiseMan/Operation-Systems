/*
 * PROJECT:         ReactOS Processes
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            drivers/processes.c
 * PURPOSE:         Processes Output Driver
 * PROGRAMMERS:     Vsevolod Molchanov (vsevolodmolchanov@gmail.com)
 */

/* INCLUDES ******************************************************************/

#include <ntddk.h>
#ifndef NDEBUG
#define NDEBUG
#endif
#include <debug.h>
#include <ntifs.h>
#include <ndk/ntndk.h>
#include <windef.h>

/* FUNCTIONS *****************************************************************/

DRIVER_UNLOAD DriverUnload;
VOID
NTAPI
DriverUnload(IN PDRIVER_OBJECT DriverObject)
{
    PDEVICE_OBJECT DeviceObject;

    /* Get DO */
    DeviceObject = DriverObject->DeviceObject;

    /* Delete the object */
    DPRINT1("------------------DRIVER-UNLOADED--------------------\n");
    IoDeleteDevice(DeviceObject);
}

NTSTATUS
NTAPI
Virtual()
{
    NTSTATUS Status;

    PVOID Pages = NULL;
    PHARDWARE_PTE PTE_BASE = (PHARDWARE_PTE)0xc0000000;
    PHARDWARE_PTE pte;
    SIZE_T sizeReserve = 1024 * 4 * 10; // bytes for 32-bit version
    SIZE_T sizeCommit = 1024 * 4 * 5; // bytes for 32-bit version
    int i;

    /* Reserving 10 pages */
    Status = ZwAllocateVirtualMemory(NtCurrentProcess(), &Pages, 0, &sizeReserve, MEM_RESERVE, PAGE_READWRITE);
    if (!NT_SUCCESS(Status)){
        DPRINT1("ERROR: Failed to reserve memory\n");
        return Status;
    }

    /* Committing first 5 pages */
    Status = ZwAllocateVirtualMemory(NtCurrentProcess(), &Pages, 0 , &sizeCommit, MEM_COMMIT, PAGE_READWRITE);
    if(!NT_SUCCESS(Status)) {
    	DPRINT1("ERROR: Failed to commit memory\n");
    	return Status;
	}

    /* Assigning values to memory allocated */
    for(i = 0; i < 5; i++){
        *((PCHAR)Pages + 0x1000 * i) = i + 1;
    }

    /* Printing informating */
	for(i = 0; i < 5; i++) {
		pte = ((ULONG)Pages >> 12) + PTE_BASE + i;
		DPRINT1("Page №%d\n\
		Physical address: %d\n\
		Valid:            %d\n\
		WriteThrough:     %d\n\
		CacheDisable:     %d\n\
		Accessed:         %d\n\
		Dirty:            %d\n\
		LargePage:        %d\n\
		Global:           %d\n\
		CopyOnWrite:      %d\n\
		Prototype:        %d\n\
		PageFrameNumber:  0x%x\n\
		\n", i + 1, pte->PageFrameNumber * 0x1000, pte->Valid,
		pte->WriteThrough, pte->CacheDisable, pte->Accessed,
		pte->Dirty, pte->LargePage, pte->Global,
		pte->CopyOnWrite, pte->Prototype, pte->PageFrameNumber);

	}

    /* Releasing memory */
    ZwFreeVirtualMemory(NtCurrentProcess(), &Pages, 0, MEM_RELEASE);

    return Status;
}

NTSTATUS
NTAPI
DriverEntry(IN PDRIVER_OBJECT DriverObject,
            IN PUNICODE_STRING RegistryPath)
{
    // PDEVICE_EXTENSION DeviceExtension;
    PDEVICE_OBJECT DeviceObject;
    UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\Virtual");
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(RegistryPath);

    /* Create the device */
    Status = IoCreateDevice(DriverObject,
                            0,
                            &DeviceName,
                            FILE_DEVICE_UNKNOWN,
                            0,
                            FALSE,
                            &DeviceObject);
    if (!NT_SUCCESS(Status)) return Status;

    DriverObject->DriverUnload = DriverUnload;

    /* Page the entire driver */
    MmPageEntireDriver(DriverEntry);

    /* Allocating virtual memory */
    Virtual();

    return STATUS_SUCCESS;
}

/* EOF */
