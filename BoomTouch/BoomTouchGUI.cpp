#include "pch.h"
#include "BoomTouch.h"

std::string BoomTouch::GetPluginName() {
	return "BoomTouch";
}


// Render the plugin settings here
// This will show up in bakkesmod when the plugin is loaded at
//  f2 -> plugins -> BoomTouch
void BoomTouch::RenderSettings() {
	ImGui::TextUnformatted("BoomTouch plugin settings");
	CVarWrapper onOff = cvarManager->getCvar("BoomTouch_On");
	CVarWrapper timeBeforeExplosion = cvarManager->getCvar("BoomTouch_TimeBeforeExplosion");
	if (onOff.IsNull()) { return; }
	if (timeBeforeExplosion.IsNull()) { return; }
	int tbe = timeBeforeExplosion.getIntValue();
	bool onoff = onOff.getBoolValue();
	if (ImGui::Checkbox("On/Off", &onoff)) {
		onOff.setValue(onoff);
	}
	if (ImGui::SliderInt("Time Before Explosion", &tbe, 0, 10)) {
		timeBeforeExplosion.setValue(tbe);
	}
}



// Do ImGui rendering here
void BoomTouch::Render()
{
	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_None))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	ImGui::End();

	if (!isWindowOpen_)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

// Name of the menu that is used to toggle the window.
std::string BoomTouch::GetMenuName()
{
	return "BoomTouch";
}

// Title to give the menu
std::string BoomTouch::GetMenuTitle()
{
	return menuTitle_;
}

// Don't call this yourself, BM will call this function with a pointer to the current ImGui context
void BoomTouch::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Should events such as mouse clicks/key inputs be blocked so they won't reach the game
bool BoomTouch::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

// Return true if window should be interactive
bool BoomTouch::IsActiveOverlay()
{
	return true;
}

// Called when window is opened
void BoomTouch::OnOpen()
{
	isWindowOpen_ = true;
}

// Called when window is closed
void BoomTouch::OnClose()
{
	isWindowOpen_ = false;
}

