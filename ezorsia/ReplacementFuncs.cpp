#pragma once

#include "stdafx.h"
#include "AutoTypes.h"
#include "Memory.h"
#include <WinSock2.h>
#include <ws2tcpip.h>
#include<iostream>
#include<fstream>
#include<sstream>
#pragma comment(lib, "ws2_32.lib")

//notes from my knowledge as i have not used these kinds of codes practically well
//function replacement is when you replace the original function in the client with your own fake function, usually to add some extra functionality
//for more complex applications you would also need to define the client's variables and reinterpret_cast those (no void this time)
//you need the right calling convention (match client's original or use _fastcall, i havent tried it much)
//it would help to know the benefits and drawbacks of "reinterpret_cast", as well as how it is hooking to prevent accidents
//hooking to the original function will replace it at all times when it is called by the client
//i personally have not tried it more because it requires a very thorough understanding of how the client code works, re-making the parts here,
//and using it, all together, in a way that doesnt break anything
//it would be the best way to do it for very extensive client edits and if you need to replace entire functions in that context but
//code caving is generally easier for short term, one-time patchwork fixes	//thanks you teto for helping me on this learning journey

bool HookGetModuleFileName(bool bEnable) {
	static decltype(&GetModuleFileNameW) _GetModuleFileNameW = &GetModuleFileNameW;

	const decltype(&GetModuleFileNameW) GetModuleFileNameW_Hook = [](HMODULE hModule, LPWSTR lpFileName, DWORD dwSize) -> DWORD {
		auto len = _GetModuleFileNameW(hModule, lpFileName, dwSize);
		// Check to see if the length is invalid (zero)
		if (!len) {
			// Try again without the provided module for a fixed result
			len = _GetModuleFileNameW(nullptr, lpFileName, dwSize);
		}
		return len;
		};

	return Memory::SetHook(bEnable, reinterpret_cast<void**>(&_GetModuleFileNameW), GetModuleFileNameW_Hook);
}

/// <summary>
/// Creates a detour for the User32.dll CreateWindowExA function applying the following changes:
/// 1. Enable the window minimize box
/// </summary>
void HookCreateWindowExA(bool bEnable) {
	static auto create_window_ex_a = decltype(&CreateWindowExA)(GetProcAddress(LoadLibraryA("USER32"), "CreateWindowExA"));
	static const decltype(&CreateWindowExA) hook = [](DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) -> HWND {
		dwStyle |= WS_MINIMIZEBOX; // enable minimize button
		//if (Client::WindowedMode)
		dwStyle &= ~WS_MAXIMIZEBOX;
		//x = (GetSystemMetrics(SM_CXSCREEN) - nWidth) / 2;
		//y = (GetSystemMetrics(SM_CYSCREEN) - nHeight) / 4;
		return create_window_ex_a(dwExStyle, lpClassName, (Client::ServerName + " " + Client::ServerNameTips).c_str(), dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		};
	Memory::SetHook(bEnable, reinterpret_cast<void**>(&create_window_ex_a), hook);
}

DWORD GetFuncAddress(LPCSTR lpModule, LPCSTR lpFunc)	//ty alias!			//multiclient, not currently working, likely cannot hook early enough with nmconew.dll
{
	HMODULE mod = LoadLibraryA(lpModule);

	if (!mod)
	{
		return 0;
	}

	DWORD address = (DWORD)GetProcAddress(mod, lpFunc);

	if (!address)
	{
		return 0;
	}

	//#ifdef _DEBUG
		//Log(__FUNCTION__ " [%s] %s @ %8X", lpModule, lpFunc, address);
	//#endif

	return address;
}

bool Hook_CreateMutexA(bool bEnable)	//ty darter	//ty angel!
{
	static auto _CreateMutexA = decltype(&CreateMutexA)(GetFuncAddress("KERNEL32", "CreateMutexA"));

	decltype(&CreateMutexA) Hook = [](LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName) -> HANDLE
		{
			//Multi-Client Check Removal
			if (lpName && strstr(lpName, "WvsClientMtx"))
			{
				return (HANDLE)0x0BADF00D;
				//char szMutex[128];
				//lpName = szMutex;
			}

			return _CreateMutexA(lpMutexAttributes, bInitialOwner, lpName);
		};

	return Memory::SetHook(bEnable, reinterpret_cast<void**>(&_CreateMutexA), Hook);
}

bool Hook_htons(bool bEnable)
{
	static auto _htons = decltype(&htons)(GetFuncAddress("WS2_32.dll", "htons"));

	decltype(&htons) Hook = [](u_short hostshort) -> u_short
		{
			return _htons(hostshort);
		};

	return Memory::SetHook(bEnable, reinterpret_cast<void**>(&_htons), Hook);
}

bool Hook_gethostbyname(bool bEnable)
{
	static auto _gethostbyname = decltype(&gethostbyname)(GetFuncAddress("WS2_32.dll", "gethostbyname"));

	decltype(&gethostbyname) Hook = [](const char* name) -> hostent*
		{
			{
				HANDLE h = CreateFileA("ijl15.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				if (h != INVALID_HANDLE_VALUE) {
					SetFilePointer(h, 0, NULL, FILE_END);
					DWORD written;
					const char* msg = "[ijl15] gethostbyname hook HIT: ";
					WriteFile(h, msg, (DWORD)strlen(msg), &written, NULL);
					WriteFile(h, name, (DWORD)strlen(name), &written, NULL);
					const char* nl = "\r\n";
					WriteFile(h, nl, 2, &written, NULL);
					CloseHandle(h);
				}
			}
			if (!Client::canInjected) {
				std::unique_lock<std::mutex> lock(Client::injected);
				if (!Client::canInjected) {
					Client::canInjected = true;
					Client::injectedCondition.notify_all();
					Client::injectedCondition.wait(lock, [] {return Client::isInjected; });
				}
				lock.unlock();
				std::cout << "Injected initialized" << std::endl;
			}
			if (strncmp(name, "mxdlogin", strlen("mxdlogin")) == 0)
			{
				return nullptr;
				//if (strncmp(name, "mxdlogin.", strlen("mxdlogin.")) == 0) {
				//	std::cout << "Hook_gethostbyname: " << name << " ignore" << std::endl;   //ingore first call
				//}
				//else {
				//	const char* serverIP_Address = Client::ServerIP_AddressFromINI.c_str();
				//	std::cout << "Hook_gethostbyname: " << name << " -> " << serverIP_Address << std::endl;
				//	return _gethostbyname(serverIP_Address);
				//}
			}
			return _gethostbyname(name);
		};

	return Memory::SetHook(bEnable, reinterpret_cast<void**>(&_gethostbyname), Hook);
}

bool Hook_connect(bool bEnable)
{
	static auto _connect = decltype(&connect)(GetFuncAddress("WS2_32.dll", "connect"));

	decltype(&connect) Hook = [](SOCKET s, const struct sockaddr* name, int namelen) -> int
		{
			struct sockaddr_in* addr = (struct sockaddr_in*)name;
			char ip_str[INET_ADDRSTRLEN];
			if (inet_ntop(AF_INET, &(addr->sin_addr), ip_str, sizeof(ip_str)) != NULL) {
				if (strcmp("255.255.255.255", ip_str) == 0) {
					ULONG ipv4_bin;
					if (inet_pton(AF_INET, Client::ServerIP_AddressFromINI.c_str(), &(ipv4_bin)) == 1) {
						addr->sin_addr.S_un.S_addr = ipv4_bin;
						addr->sin_port = htons(Client::serverIP_Port);
					}
				}
			}
			return _connect(s, name, namelen);
		};

	return Memory::SetHook(bEnable, reinterpret_cast<void**>(&_connect), Hook);
}

bool Hook_StringPool__GetString(bool bEnable)	//hook stringpool modification //ty !! popcorn //ty darter
{
	_StringPool__GetString_t _StringPool__GetString_Hook = [](void* pThis, void* edx, ZXString<char>* result, unsigned int nIdx, char formal) ->  ZXString<char>*
		{
			auto ret = _StringPool__GetString(pThis, edx, result, nIdx, formal);
			auto str = static_cast<const char*>(*ret);
			//if (strstr(str, "uiOpt") != nullptr) {
			//	std::cout << "StringPool__GetString id = " << nIdx << " " << str << " " << _ReturnAddress() << std::endl;
			//}
			switch (nIdx)
			{
			case 737:
				*ret = Client::WelcomeMessage.c_str();
				break;
			case 2219:
				*ret = "soResolution";
				break;
			case 2224:
				*ret = "wuiOpt%d";
				break;
				//case 233:
					//if(Client::longEXP)
					//	*ret = ("�õ�����ֵ (+%ld)"); break;
					//case 1307:	//1307_UI_LOGINIMG_COMMON_FRAME = 51Bh
					//	if (EzorsiaV2WzIncluded && !ownLoginFrame) {
					//		switch (Client::m_nGameWidth)
					//		{
					//			case 1280:	//ty teto for the suggestion to use ZXString<char>::Assign and showing me available resources
					//				*ret = ("UI/MapleEzorsiaV2wzfiles.img/Common/frame1280"); break;
					//			case 1366:
					//				*ret = ("UI/MapleEzorsiaV2wzfiles.img/Common/frame1366"); break;
					//			case 1600:
					//				*ret = ("UI/MapleEzorsiaV2wzfiles.img/Common/frame1600"); break;
					//			case 1920:
					//				*ret = ("UI/MapleEzorsiaV2wzfiles.img/Common/frame1920"); break;
					//			case 1024:
					//				*ret = ("UI/MapleEzorsiaV2wzfiles.img/Common/frame1024"); break;
					//		}
					//		break;
					//	}
					//case 1301:	//1301_UI_CASHSHOPIMG_BASE_BACKGRND  = 515h
					//	if (EzorsiaV2WzIncluded && !ownCashShopFrame) { *ret = ("UI/MapleEzorsiaV2wzfiles.img/Base/backgrnd"); } break;
					//case 1302:	//1302_UI_CASHSHOPIMG_BASE_BACKGRND1 = 516h
					//	if (EzorsiaV2WzIncluded && !ownCashShopFrame) { *ret = ("UI/MapleEzorsiaV2wzfiles.img/Base/backgrnd1"); } break;
					//case 5361:	//5361_UI_CASHSHOPIMG_BASE_BACKGRND2  = 14F1h			
					//	if (EzorsiaV2WzIncluded && !ownCashShopFrame) { *ret = ("UI/MapleEzorsiaV2wzfiles.img/Base/backgrnd2"); } break;
					////case 1302:	//BACKGRND??????
					////	if (EzorsiaV2WzIncluded && ownCashShopFrame) { *ret = ("UI/MapleEzorsiaV2wzfiles.img/Base/backgrnd1"); } break;
					////case 5361:	//SP_1937_UI_UIWINDOWIMG_STAT_BACKGRND2  = 791h	
					////	if (EzorsiaV2WzIncluded && ownCashShopFrame) { *ret = ("UI/MapleEzorsiaV2wzfiles.img/Base/backgrnd2"); } break;
					//default:
					//	if (Client::SwitchChinese)
					//	{
					//		for (const auto& pair : newKeyValuePairs) {
					//			if (nIdx == pair.key) {
					//				*ret = pair.value.c_str();
					//				break;
					//			}
					//		}
					//	}
					//	break;
			}
			return ret;
		};
	return Memory::SetHook(bEnable, reinterpret_cast<void**>(&_StringPool__GetString), _StringPool__GetString_Hook);
}
bool Hook_StringPool__GetStringW(bool bEnable)	//hook stringpool modification //ty !! popcorn //ty darter
{
	_StringPool__GetStringW_t _StringPool__GetStringW_Hook = [](void* pThis, void* edx, ZXString<wchar_t>* result, unsigned int nIdx, char formal) ->  ZXString<wchar_t>*
		{
			auto ret = _StringPool__GetStringW(pThis, edx, result, nIdx, formal);
			//const wchar_t* str = static_cast<const wchar_t*>(*ret);
			//if (wcsstr(str, L"uiOpt") != nullptr) {
			//	std::wcout << "StringPool__GetStringW id = " << nIdx << " " << str << " " << _ReturnAddress() << std::endl;
			//}
			//if (nIdx == 1163)
			//{
			//    *ret = "BeiDou";
			//}
			switch (nIdx)
			{
				//case 1307:	//1307_UI_LOGINIMG_COMMON_FRAME = 51Bh
				//	if (EzorsiaV2WzIncluded && !ownLoginFrame) {
				//		switch (Client::m_nGameWidth)
				//		{
				//			case 1280:	//ty teto for the suggestion to use ZXString<char>::Assign and showing me available resources
				//				*ret = ("UI/MapleEzorsiaV2wzfiles.img/Common/frame1280"); break;
				//			case 1366:
				//				*ret = ("UI/MapleEzorsiaV2wzfiles.img/Common/frame1366"); break;
				//			case 1600:
				//				*ret = ("UI/MapleEzorsiaV2wzfiles.img/Common/frame1600"); break;
				//			case 1920:
				//				*ret = ("UI/MapleEzorsiaV2wzfiles.img/Common/frame1920"); break;
				//			case 1024:
				//				*ret = ("UI/MapleEzorsiaV2wzfiles.img/Common/frame1024"); break;
				//		}
				//		break;
				//	}
				//case 1301:	//1301_UI_CASHSHOPIMG_BASE_BACKGRND  = 515h
				//	if (EzorsiaV2WzIncluded && !ownCashShopFrame) { *ret = ("UI/MapleEzorsiaV2wzfiles.img/Base/backgrnd"); } break;
				//case 1302:	//1302_UI_CASHSHOPIMG_BASE_BACKGRND1 = 516h
				//	if (EzorsiaV2WzIncluded && !ownCashShopFrame) { *ret = ("UI/MapleEzorsiaV2wzfiles.img/Base/backgrnd1"); } break;
				//case 5361:	//5361_UI_CASHSHOPIMG_BASE_BACKGRND2  = 14F1h			
				//	if (EzorsiaV2WzIncluded && !ownCashShopFrame) { *ret = ("UI/MapleEzorsiaV2wzfiles.img/Base/backgrnd2"); } break;
				////case 1302:	//BACKGRND??????
				////	if (EzorsiaV2WzIncluded && ownCashShopFrame) { *ret = ("UI/MapleEzorsiaV2wzfiles.img/Base/backgrnd1"); } break;
				////case 5361:	//SP_1937_UI_UIWINDOWIMG_STAT_BACKGRND2  = 791h	
				////	if (EzorsiaV2WzIncluded && ownCashShopFrame) { *ret = ("UI/MapleEzorsiaV2wzfiles.img/Base/backgrnd2"); } break;
				//default:
				//	if (Client::SwitchChinese)
				//	{
				//		for (const auto& pair : newKeyValuePairs) {
				//			if (nIdx == pair.key) {
				//				*ret = pair.value.c_str();
				//				break;
				//			}
				//		}
				//	}
				//	break;
			}
			return ret;
		};
	return Memory::SetHook(bEnable, reinterpret_cast<void**>(&_StringPool__GetStringW), _StringPool__GetStringW_Hook);
}

bool Hook_CItemInfo__GetItemName(bool enable)
{
	CItemInfo__GetItemName_t hook = [](void* pThis, void* edx, ZXString<char>* result, int nItemID) -> ZXString<char>*
		{
			auto ret = CItemInfo__GetItemName(pThis, edx, result, nItemID);

			int type = nItemID / 1000000;
			if (type >= 1 && type <= 5)
			{
				*ret += "(";
				*ret += std::to_string(nItemID).c_str();
				*ret += ")";
			}

			return ret;
		};
	return  Memory::SetHook(enable, reinterpret_cast<void**>(&CItemInfo__GetItemName), hook);
}

bool Hook_CItemInfo__GetItemDesc(bool enable)
{
	CItemInfo__GetItemDesc_t hook = [](void* pThis, void* edx, ZXString<char>* result, int nItemID) -> ZXString<char>*
		{
			auto ret = CItemInfo__GetItemDesc(pThis, edx, result, nItemID);

			int type = nItemID / 1000000;
			if (type >= 1 && type <= 5)
			{
				std::stringstream ss;
				ss << "ID: ";
				ss << std::to_string(nItemID);
				if (ret->Length() > 0)
				{
					ss << "\r\n";
					ss << static_cast<const char*>(*ret);
				}
				std::string s = ss.str();
				ret->Assign(s.c_str(), s.length());
			}

			return ret;
		};
	return  Memory::SetHook(enable, reinterpret_cast<void**>(&CItemInfo__GetItemDesc), hook);
}

int __fastcall _Sub_8C9F01_t(DWORD* pThis, void* edx)
{
	//auto ret = Sub_8C9F01(pThis, edx);
	if (Sub_8C9F38(pThis, edx)) {
		auto mobId = *(int*)Sub_55A8C1((DWORD*)(*(pThis + 0x2FF)), edx);
		if (mobId == 1932000 && (int)_ReturnAddress() >= 0x00974307 && (int)_ReturnAddress() <= 0x009762FE)
			return 0;
		//std::cout << "_Sub_8C9F01_t:" << " mobId;" << mobId << " " << _ReturnAddress() << std::endl;
		return 1;
	}
	return 0;
};

bool Hook_Sub_8C9F01(bool enable)
{
	return  Memory::SetHook(enable, reinterpret_cast<void**>(&Sub_8C9F01), _Sub_8C9F01_t);
}