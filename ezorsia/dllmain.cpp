// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "ijl15.h"
#include "INIReader.h"
#include "ReplacementFuncs.h"
#include <comutil.h>
#include <windows.h>
#include <stdio.h>
#include "BossHP.h"
#include <Resman.h>
#include <CharacterEx.h>
#include "ChairRelMove.h"
#include <HeapCreateEx.h>
#include "dllmain.h"
#include <InlinkOutlink.h>
#include "CreateDump.h"
#include "AutoDump.h"

// Simple file logger for diagnostics
static void LogIjl15(const char* msg) {
    HANDLE h = CreateFileA("ijl15.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) return;
    SetFilePointer(h, 0, NULL, FILE_END);
    DWORD written;
    DWORD len = (DWORD)strlen(msg);
    WriteFile(h, msg, len, &written, NULL);
    const char* nl = "\r\n";
    WriteFile(h, nl, 2, &written, NULL);
    CloseHandle(h);
}

CAutoDump* autoDump = nullptr;

void Init()
{
	LogIjl15("[ijl15] Init() started");

	INIReader reader("config.ini");
	if (reader.ParseError() == 0) {
		LogIjl15("[ijl15] config.ini parsed OK");
		Client::DefaultResolution = reader.GetInteger("general", "DefaultResolution", Client::DefaultResolution);
		char buf[64];
		sprintf_s(buf, "[ijl15] DefaultResolution=%d", (int)Client::DefaultResolution);
		LogIjl15(buf);
		Client::MsgAmount = reader.GetInteger("general", "MsgAmount", Client::MsgAmount);
		Client::WindowedMode = reader.GetBoolean("general", "WindowedMode", Client::WindowedMode);
		Client::RemoveLogos = reader.GetBoolean("general", "RemoveLogos", Client::RemoveLogos);
		Client::SkipWorldSelect = reader.GetBoolean("general", "SkipWorldSelect", Client::SkipWorldSelect);
		Client::RemoveLoginNxIdDialog = reader.GetBoolean("general", "RemoveLoginNxIdDialog", Client::RemoveLoginNxIdDialog);
		Client::ScreenShotPath = reader.GetBoolean("general", "ScreenShotPath", Client::ScreenShotPath);
		Client::RemoveSystemMsg = reader.GetBoolean("general", "RemoveSystemMsg", Client::RemoveSystemMsg);
		Memory::UseVirtuProtect = reader.GetBoolean("general", "UseVirtuProtect", Memory::UseVirtuProtect);
		Client::ResCheckTime = reader.GetInteger("general", "ResCheckTime", Client::ResCheckTime);
		Client::ResFlushTimeInterval = reader.GetInteger("general", "ResFlushTimeInterval", Client::ResFlushTimeInterval);
		Client::ResManFlushCached = reader.GetInteger("general", "ResManFlushCached", Client::ResManFlushCached);
		Client::SetWorkingSetSize = reader.GetInteger("general", "SetWorkingSetSize", Client::SetWorkingSetSize);
		//Client::setDamageCap = reader.GetReal("optional", "setDamageCap", Client::setDamageCap);//�﹥���
		//Client::setMAtkCap = reader.GetReal("optional", "setMAtkCap", Client::setMAtkCap);//ħ��ħ�����
		Client::setAccCap = reader.GetReal("optional", "setAccCap", Client::setAccCap);
		Client::setAvdCap = reader.GetReal("optional", "setAvdCap", Client::setAvdCap);
		Client::setAtkOutCap = reader.GetReal("optional", "setAtkOutCap", Client::setAtkOutCap);
		Client::longEXP = reader.GetBoolean("optional", "longEXP", Client::longEXP);
		Client::levelExpOverride = reader.GetString("optional", "levelExpOverride", Client::levelExpOverride);
		Client::levelType = reader.GetInteger("optional", "levelType", Client::levelType);
		Client::useTubi = reader.GetBoolean("optional", "useTubi", Client::useTubi);
		Client::speedMovementCap = reader.GetInteger("optional", "speedMovementCap", Client::speedMovementCap);
		Client::jumpCap = reader.GetInteger("optional", "jumpCap", Client::jumpCap);
		Client::debug = reader.GetBoolean("debug", "debug", Client::debug);
		Client::crashAutoDump = reader.GetBoolean("debug", "crashAutoDump", Client::crashAutoDump);
		Client::noPassword = reader.GetBoolean("debug", "noPassword", Client::noPassword);
		Client::forceExit = reader.GetBoolean("debug", "forceExit", Client::forceExit);
		Client::linkNodeNew = reader.GetBoolean("debug", "linkNodeNew", Client::linkNodeNew);
		Client::ServerName = reader.GetString("general", "ServerName", Client::ServerName);
		Client::ServerNameTips = reader.GetString("general", "ServerNameTips", Client::ServerNameTips);
		Client::WelcomeMessage = reader.GetString("general", "WelcomeMessage", Client::WelcomeMessage);
		Client::ServerIP_AddressFromINI = reader.GetString("general", "ServerIP_Address", Client::ServerIP_AddressFromINI);
		Client::ServerIP_Address_hook = reader.GetBoolean("general", "ServerIP_Address_hook", Client::ServerIP_Address_hook);
		Client::climbSpeedAuto = reader.GetBoolean("optional", "climbSpeedAuto", Client::climbSpeedAuto);
		Client::climbSpeed = reader.GetReal("optional", "climbSpeed", Client::climbSpeed);
		Client::serverIP_Port = reader.GetInteger("general", "serverIP_Port", Client::serverIP_Port);
		Client::talkRepeat = reader.GetBoolean("optional", "talkRepeat", Client::talkRepeat);
		Client::talkTime = reader.GetInteger("optional", "talkTime", Client::talkTime);
		Client::meleePunching = reader.GetBoolean("optional", "meleePunching", Client::meleePunching);
		Client::holdAttack = reader.GetBoolean("optional", "holdAttack", Client::holdAttack);
		Client::spLimit = reader.GetBoolean("optional", "spLimit", Client::spLimit);
		Client::tamingMobUnlock = reader.GetBoolean("optional", "tamingMobUnlock", Client::tamingMobUnlock);
		Client::replacePetEquipCheck = reader.GetBoolean("optional", "replacePetEquipCheck", Client::replacePetEquipCheck);
		Client::downJumpLimitHeight = reader.GetInteger("optional", "downJumpLimitHeight", Client::downJumpLimitHeight);
		Client::unlockPanelLimit = reader.GetBoolean("optional", "unlockPanelLimit", Client::unlockPanelLimit);
		Client::unlockPaneMaplLimit = reader.GetBoolean("optional", "unlockPaneMaplLimit", Client::unlockPaneMaplLimit);
		Client::longSlots = reader.GetBoolean("ui", "LongSlots", Client::longSlots);
		Client::longSlotsKey = reader.GetInteger("ui", "longSlotsKey", Client::longSlotsKey);
		Client::showItemID = reader.GetBoolean("ui", "showItemID", Client::showItemID);
		Client::showWeaponSpeed = reader.GetBoolean("ui", "showWeaponSpeed", Client::showWeaponSpeed);
		Client::minimizeMaptitleColor = reader.GetBoolean("ui", "minimizeMaptitleColor", Client::minimizeMaptitleColor);
		Client::StatDetailBackgrndWidthEx = reader.GetInteger("ui", "StatDetailBackgrndWidthEx", Client::StatDetailBackgrndWidthEx);
		Client::DamageSkin = reader.GetInteger("ui", "DamageSkin", Client::DamageSkin);
		Client::RemoteDamageSkin = reader.GetBoolean("ui", "RemoteDamageSkin", Client::RemoteDamageSkin);
		Client::tamingMob198Effect = reader.GetBoolean("ui", "tamingMob198", Client::tamingMob198Effect);
		Client::s2221006 = reader.GetBoolean("skill", "s2221006", Client::s2221006);
		Client::s4221001 = reader.GetBoolean("skill", "s4221001", Client::s4221001);
		Client::s4221007 = reader.GetBoolean("skill", "s4221007", Client::s4221007);
		Client::s14101004 = reader.GetBoolean("skill", "s14101004", Client::s14101004);
		Client::s14101004up = reader.GetBoolean("skill", "s14101004up", Client::s14101004up);
		Client::s5221009 = reader.GetBoolean("skill", "s5221009", Client::s5221009);
		Client::s2100NoMove = reader.GetBoolean("skill", "s2100NoMove", Client::s2100NoMove);
	}
	if (Client::debug)
		CreateConsole();	//console for devs, use this to log stuff if you want
	if (Client::crashAutoDump) {
		autoDump = new CAutoDump();
	}
	CheckMonitorRefreshRate();
	// DISABLED: conflicts with Hook.dll's AntiCheat removal + MultiClient
	// Hook_CreateMutexA(true);
	// DISABLED: conflicts with Hook.dll's CreateWindowExA hook
	// HookCreateWindowExA(true);
	HeapCreateEx::HOOK_HeapCreate();
	Hook_gethostbyname(true);
	LogIjl15("[ijl15] Hook_gethostbyname installed");
	Hook_connect(Client::ServerIP_Address_hook);
	TimerTask(CreateHook, 10);
	LogIjl15("[ijl15] Init() done, TimerTask started");
}

void CreateConsole() {
	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w", stdout); //CONOUT$
}

bool CreateHook()
{
	LogIjl15("[ijl15] CreateHook() called");
	if (Client::isInjected)
	{
		LogIjl15("[ijl15] CreateHook() already injected, returning");
		Client::injectedCondition.notify_all();
		return true;
	}
	LogIjl15("[ijl15] CreateHook() waiting for canInjected");
	std::unique_lock<std::mutex> lock(Client::injected);
	if (!Client::canInjected) {
		std::cout << "CreateHook invoke wait can inject" << std::endl;
		LogIjl15("[ijl15] waiting on condition variable...");
		Client::injectedCondition.wait(lock, [] {return Client::canInjected; });
	}
	LogIjl15("[ijl15] CreateHook() proceed past wait, starting injection");
	std::cout << "CreateHook invoke inject start" << std::endl;
	Hook_StringPool__GetString(true); //hook stringpool modification //ty !! popcorn //ty darter
	Hook_StringPool__GetStringW(true);
	std::string processName = GetCurrentProcessName();
	std::cout << "Current process name: " << processName << std::endl;
	Client::CRCBypass();
	//Client::EnableNewIGCipher();
	Resman::Hook_InitializeResMan();
	Resman::Hook_InitGetHook();
	InlinkOutlink::AttachClientInlink(Client::linkNodeNew);
	Hook_SetActiveChairRelMove(true);
	CharacterEx::Init();
	HeapCreateEx::MemoryOptimization();
	Client::UpdateGameStartup();
	//Client::LongQuickSlot();
	Client::FixMouseWheel();
	Client::JumpCap();
	Client::NoPassword();
	BossHP::Hook();
	Client::MoreHook();
	Client::Skill();
	//Hook_CItemInfo__GetItemName(Client::showItemID);
	Hook_CItemInfo__GetItemDesc(Client::showItemID);
	ijl15::CreateHook(); //NMCO::CreateHook();
	TimerTask(Client::EmptyMemory, Client::ResCheckTime);
	std::cout << "CreateHook invoke inject successed" << std::endl;
	LogIjl15("[ijl15] CreateHook() injection SUCCESS");
	Client::isInjected = true;
	Client::injectedCondition.notify_all();
	lock.unlock();
	return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hModule);
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
	{
		LogIjl15("[ijl15] DllMain DLL_PROCESS_ATTACH");
		Init();
		break;
	}
	default: break;
	case DLL_PROCESS_DETACH:
		if (autoDump) {
			delete autoDump;
			autoDump = nullptr;
		}
		ExitProcess(0);
	}
	return TRUE;
}