#pragma once
#include <ntddk.h>

#define PROCESS_MEM_TAG 'corP'
#define MAX_PATH 256


typedef struct _PROCESS_INFO
{
public:
    NTSTATUS Initialize(PPS_CREATE_NOTIFY_INFO CreateInfo, HANDLE ProcessId)
	{
        PId_ = ProcessId;
        PPId_ = CreateInfo->ParentProcessId;
        CTId_ = CreateInfo->CreatingThreadId.UniqueThread;
        CPId_ = CreateInfo->CreatingThreadId.UniqueProcess;
        CmdLine_ = NULL;
        ImageFileName_ = NULL;
        NTSTATUS Status = STATUS_SUCCESS;

        // Allocate image name and command line
        if (CreateInfo->ImageFileName)
        {
            ImageFileName_ = (PUNICODE_STRING)ExAllocatePoolWithTag(NonPagedPool, ((CreateInfo->ImageFileName->Length + 1) * sizeof(WCHAR)) + sizeof(UNICODE_STRING), PROCESS_MEM_TAG);
            if (!ImageFileName_)
            {
                Status = STATUS_NO_MEMORY;
                DbgError("Failed allocating memory for process info (PID: %u)", HandleToULong(ProcessId));
                return Status;
            }
            PWCHAR ImageNameBuffer = (PWCHAR)((PUCHAR)ImageFileName_ + sizeof(UNICODE_STRING));
            RtlCopyMemory(ImageNameBuffer,
                CreateInfo->ImageFileName->Buffer,
                CreateInfo->ImageFileName->Length + 1);
            RtlInitUnicodeString(ImageFileName_, ImageNameBuffer);
        }

        if (CreateInfo->CommandLine)
        {

            CmdLine_ = (PUNICODE_STRING)ExAllocatePoolWithTag(NonPagedPool, ((CreateInfo->CommandLine->Length + 1) * sizeof(WCHAR)) + sizeof(UNICODE_STRING), PROCESS_MEM_TAG);
            if (!CmdLine_)
            {
                Status = STATUS_NO_MEMORY;
                ExFreePoolWithTag(ImageFileName_, PROCESS_MEM_TAG);
                DbgError("Failed allocating memory for process info (PID: %u)", HandleToULong(ProcessId));
                return Status;
            }

            PWCHAR CmdLineBuffer = (PWCHAR)((PUCHAR)CmdLine_ + sizeof(UNICODE_STRING));
            RtlCopyMemory(CmdLineBuffer,
                CreateInfo->CommandLine->Buffer,
                CreateInfo->CommandLine->Length);
            RtlInitUnicodeString(CmdLine_, CmdLineBuffer);
        }

        return Status;
	}

    VOID FreeInternalMemory()
    {
        if(ImageFileName_)
            ExFreePoolWithTag(ImageFileName_, PROCESS_MEM_TAG);
        if(CmdLine_)
            ExFreePoolWithTag(CmdLine_, PROCESS_MEM_TAG);
    }

	HANDLE PId_;					// Process id
	HANDLE PPId_;				    // Parent process id
	HANDLE CPId_;				    // Creating process id
	HANDLE CTId_;				    // Creating thread id
	PUNICODE_STRING CmdLine_;	    // Command line
    PUNICODE_STRING ImageFileName_; // Image file name

}PROCESS_INFO, *PPROCESS_INFO;