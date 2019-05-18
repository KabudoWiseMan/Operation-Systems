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
#include <ndk/exfuncs.h>

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

VOID
NTAPI
PrintProcesses(IN PSYSTEM_PROCESS_INFORMATION ProcInfo)
{
    DPRINT1("------------------PROCESSES-LIST--------------------\n");
    while (ProcInfo->NextEntryOffset) {
        ProcInfo = (PSYSTEM_PROCESS_INFORMATION)((ULONG_PTR)ProcInfo + (ULONG_PTR)ProcInfo->NextEntryOffset);
        DPRINT1("Image name: %wZ, Unique PID: %d, Inherited PID: %d, Base priority: %d\n", &ProcInfo->ImageName, ProcInfo->UniqueProcessId, ProcInfo->InheritedFromUniqueProcessId, ProcInfo->BasePriority);
    }
}

NTSTATUS
NTAPI
Processes()
{
    NTSTATUS Status;
    PSYSTEM_PROCESS_INFORMATION ProcInfo;

    /* Creating buffer */
    ULONG BufferSize = sizeof(SYSTEM_PROCESS_INFORMATION);
    ULONG ReturnLength;
    PVOID Buffer = ExAllocatePool(NonPagedPool, BufferSize);
    if (Buffer == NULL)
    {
        DPRINT1("ERROR: NOT ENOUGH MEMORY\n");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    /* Getting processes information */
    Status = ZwQuerySystemInformation(SystemProcessInformation, Buffer, BufferSize, &ReturnLength);

    /* Reallocating buffer if not it's enough and getting process information again */
    if (Status == STATUS_INFO_LENGTH_MISMATCH)
    {
		ExFreePool(Buffer);
        BufferSize = ReturnLength;
        Buffer = ExAllocatePool(NonPagedPool, BufferSize);
        if (Buffer == NULL)
        {
            DPRINT1("ERROR: NOT ENOUGH MEMORY\n");
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        Status = ZwQuerySystemInformation(SystemProcessInformation, Buffer, BufferSize, &ReturnLength);
	}
    if (!NT_SUCCESS(Status))
    {
      ExFreePool(Buffer);
      DPRINT1("ERROR: Status = %Х\n", Status);
      return Status;
    }

    /* Printing processes information */
    ProcInfo = (PSYSTEM_PROCESS_INFORMATION)Buffer;
    PrintProcesses(ProcInfo);

    /* Freeing buffer */
    ExFreePool(Buffer);

    return Status;
}

NTSTATUS
NTAPI
DriverEntry(IN PDRIVER_OBJECT DriverObject,
            IN PUNICODE_STRING RegistryPath)
{
    // PDEVICE_EXTENSION DeviceExtension;
    PDEVICE_OBJECT DeviceObject;
    UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\Processes");
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

    /* Write all running processes */
    Processes();

    return STATUS_SUCCESS;
}

/* EOF */
