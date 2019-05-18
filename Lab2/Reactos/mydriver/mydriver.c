/*
 * PROJECT:         ReactOS Name
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            drivers/mydriver.c
 * PURPOSE:         Name Output Driver
 * PROGRAMMERS:     Vsevolod Molchanov (vsevolodmolchanov@gmail.com)
 */

/* INCLUDES ******************************************************************/

#include <ntddk.h>
#ifndef NDEBUG
#define NDEBUG
#endif
#include <debug.h>

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
DriverEntry(IN PDRIVER_OBJECT DriverObject,
            IN PUNICODE_STRING RegistryPath)
{
    // PDEVICE_EXTENSION DeviceExtension;
    PDEVICE_OBJECT DeviceObject;
    UNICODE_STRING DeviceName = RTL_CONSTANT_STRING(L"\\Device\\Mydriver");
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
    DPRINT1("------------------VSEVOLOD--------------------\n");
    return STATUS_SUCCESS;
}

/* EOF */
