
#include "XStorage.h"

extern void Check_Overlapped(PXOVERLAPPED pOverlapped);

// #5305: XStorageUploadFromMemory
DWORD WINAPI XStorageUploadFromMemory(DWORD dwUserIndex, const WCHAR *wszServerPath, DWORD dwBufferSize, const BYTE *pbBuffer, PXOVERLAPPED pOverlapped)
{
	LOG_TRACE_XLIVE(L"XStorageUploadFromMemory  ( wszServerPath = {}, dwBufferSize = {} )",
		wszServerPath, dwBufferSize);

	FILE *fp;
	errno_t err = _wfopen_s(&fp, wszServerPath, L"wb");
	if (err)
	{
		//LOG_TRACE_XLIVE(" - file copy failure, error: {}", err);

		if (pOverlapped)
		{
			pOverlapped->InternalHigh = 0;
			pOverlapped->InternalLow = ERROR_FUNCTION_FAILED;
			pOverlapped->dwExtendedError = HRESULT_FROM_WIN32(ERROR_FUNCTION_FAILED);

			Check_Overlapped(pOverlapped);

			return ERROR_IO_PENDING;
		}

		return ERROR_FUNCTION_FAILED;
	}

	fseek(fp, 0, SEEK_SET);
	fwrite(pbBuffer, dwBufferSize, 1, fp);
	fseek(fp, 0, SEEK_END);

	LOG_TRACE_XLIVE(L" - Uploaded total byte count: {}", ftell(fp));

	fclose(fp);

	if (pOverlapped)
	{
		pOverlapped->InternalLow = ERROR_SUCCESS;
		pOverlapped->InternalHigh = dwBufferSize;
		pOverlapped->dwExtendedError = HRESULT_FROM_WIN32(ERROR_SUCCESS);

		Check_Overlapped(pOverlapped);

		return ERROR_IO_PENDING;
	}

	return ERROR_SUCCESS;
}

// #5344: XStorageBuildServerPath
DWORD WINAPI XStorageBuildServerPath(DWORD dwUserIndex, XSTORAGE_FACILITY StorageFacility,
	const void *pvStorageFacilityInfo, DWORD dwStorageFacilityInfoSize,
	LPCWSTR *pwszItemName, WCHAR *pwszServerPath, DWORD *pdwServerPathLength)
{
	LOG_TRACE_XLIVE(L"XStorageBuildServerPath  ( StorageFacility = {},  dwStorageFacilityInfoSize = {}, pwszItemName = {}, pwszServerPath = {}, pdwServerPathLength = {} )",
		(int)StorageFacility, dwStorageFacilityInfoSize, (wchar_t*)pwszItemName, (wchar_t*)pwszServerPath, *pdwServerPathLength);

	bool PathValid = false;
	bool CreateDirReturn = false;

	DWORD fileAtributes = 0;
	std::wstring itemName((wchar_t*)pwszItemName);

	if (pwszServerPath)
	{
		std::wstring filePath(_wgetenv(L"USERPROFILE"));
		filePath += L"\\AppData\\Local";

		fileAtributes = GetFileAttributes(filePath.c_str());
		if (fileAtributes != INVALID_FILE_ATTRIBUTES && fileAtributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			filePath += L"\\Microsoft\\Halo 2";
			CreateDirReturn = CreateDirectory(filePath.c_str(), NULL);
			if (CreateDirReturn || GetLastError() == ERROR_ALREADY_EXISTS)
			{
				filePath += L"\\XLive";
				CreateDirReturn = CreateDirectory(filePath.c_str(), NULL);
				if (CreateDirReturn || GetLastError() == ERROR_ALREADY_EXISTS)
				{
					switch (StorageFacility)
					{
					case XSTORAGE_FACILITY_GAME_CLIP:
					case XSTORAGE_FACILITY_PER_TITLE:
					case XSTORAGE_FACILITY_PER_USER_TITLE:
						filePath += L"\\" + std::to_wstring(usersSignInInfo[dwUserIndex].xuid);
						break;

					default:
						return ERROR_INVALID_PARAMETER;
					}

					CreateDirReturn = CreateDirectory(filePath.c_str(), NULL);
					if (CreateDirReturn || GetLastError() == ERROR_ALREADY_EXISTS)
					{
						filePath += L"\\" + itemName;
						PathValid = true;
					}
				}
			}
		}

		if (PathValid)
		{
			*pdwServerPathLength = filePath.size();
			wcsncpy(pwszServerPath, filePath.c_str(), filePath.size());
			return ERROR_SUCCESS;
		}
	}

	return ERROR_FUNCTION_FAILED;
}


// #5345: XStorageDownloadToMemory
DWORD WINAPI XStorageDownloadToMemory(DWORD dwUserIndex,
	const WCHAR *wszServerPath,
	DWORD dwBufferSize,
	const BYTE *pbBuffer,
	DWORD cbResults,
	XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS *pResults,
	XOVERLAPPED *pXOverlapped
)
{
	LOG_TRACE_XLIVE(L"XStorageDownloadToMemory  ( wszServerPath = {}, dwBufferSize = {}, cbResults = {} )",
		wszServerPath, dwBufferSize, cbResults);

	memset(pResults, 0, sizeof(XSTORAGE_DOWNLOAD_TO_MEMORY_RESULTS));

	DWORD size = 0;
	FILE *fp = nullptr;

	FILETIME fileTime;
	SYSTEMTIME systemTime;

	GetSystemTime(&systemTime);
	SystemTimeToFileTime(&systemTime, &fileTime);

	errno_t err = _wfopen_s(&fp, wszServerPath, L"rb");
	if (err)
	{
		LOG_TRACE_XLIVE("- ERROR: file does not exist");

		if (pXOverlapped)
		{
			pXOverlapped->InternalHigh = 0;
			pXOverlapped->InternalLow = (XONLINE_E_STORAGE_FILE_NOT_FOUND & 0xFFFF);
			pXOverlapped->dwExtendedError = XONLINE_E_STORAGE_FILE_NOT_FOUND;

			return ERROR_IO_PENDING;
		}

		return XONLINE_E_STORAGE_FILE_NOT_FOUND;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);

	if (dwBufferSize < size)
	{
		LOG_TRACE_XLIVE("- ERROR_INSUFFICIENT_BUFFER = {}", ftell(fp));

		fclose(fp);

		if (pXOverlapped)
		{
			pXOverlapped->InternalHigh = 0;
			pXOverlapped->InternalLow = ERROR_INSUFFICIENT_BUFFER;
			pXOverlapped->dwExtendedError = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

			return ERROR_IO_PENDING;
		}

		return ERROR_INSUFFICIENT_BUFFER;
	}

	fseek(fp, 0, SEEK_SET);
	fread((void *)pbBuffer, size, 1, fp);

	pResults->dwBytesTotal = size;
	pResults->ftCreated = fileTime;
	pResults->xuidOwner = usersSignInInfo[dwUserIndex].xuid;

	LOG_TRACE_XLIVE("- Read total bytes = {}", pResults->dwBytesTotal);

	fclose(fp);

	if (pXOverlapped)
	{
		pXOverlapped->InternalHigh = size;
		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = HRESULT_FROM_WIN32(ERROR_SUCCESS);

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}

	return ERROR_SUCCESS;
}

// 5308
DWORD WINAPI XStorageDelete(DWORD dwUserIndex, const WCHAR *wszServerPath, XOVERLAPPED *pXOverlapped)
{
	LOG_TRACE_XLIVE(L"XStorageDelete  (*** checkme ***) (a1 = {0:x}, a2 = {1}, a3 = {2:p})",
		dwUserIndex, wszServerPath, (void*)pXOverlapped);

	DeleteFile(wszServerPath);

	if (pXOverlapped) {
		//asynchronous

		pXOverlapped->InternalHigh = 0;
		pXOverlapped->InternalLow = ERROR_SUCCESS;
		pXOverlapped->dwExtendedError = HRESULT_FROM_WIN32(ERROR_SUCCESS);

		Check_Overlapped(pXOverlapped);

		return ERROR_IO_PENDING;
	}

	return ERROR_SUCCESS;
}

// #5306: XStorageEnumerate
int WINAPI XStorageEnumerate(DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD)   // XStorageEnumerate
{
	LOG_TRACE_XLIVE("XStorageEnumerate");
	return 0;
}

// 5307
DWORD WINAPI XStorageDownloadToMemoryGetProgress(DWORD a1, DWORD a2, DWORD a3, DWORD a4)
{
	LOG_TRACE_XLIVE("XStorageDownloadToMemoryGetProgress  (*** checkme ***) (a1 = {0:x}, a2 = {1:x}, a3 = {2:x}, a4 = {3:x})",
		a1, a2, a3, a4);

	// not done - error now
	return ERROR_INVALID_PARAMETER;
}

// #5309: XStorageBuildServerPathByXuid
int WINAPI XStorageBuildServerPathByXuid(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8)
{
	LOG_TRACE_XLIVE("XStorageBuildServerPathByXuid  (a1 = {0:x}, a2 = {1:x}, a3 = {2:x}, a4 = {3:x}, a5 = {4:x}, a6 = {5:x}, a7 = {6:x}, a8 = {7:x}",
		a1, a2, a3, a4, a5, a6, a7, a8);

	return 0;
}
