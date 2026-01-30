// This file is part of Notepad++ project
// Copyright (C) 2024 Notepad++ Team

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Linux/Qt implementation of NppDarkMode
// This is a stub implementation for the Linux port where dark mode
// handling is managed by Qt's native style system

#include "NppDarkMode.h"
#include "Parameters.h"

#include <cmath>

namespace NppDarkMode
{
    // Static options storage
    static Options _options;
    static ::AdvancedOptions g_advOptions;
    static ColorTone g_colorToneChoice = blackTone;

    // Default colors for dark mode (same as Windows version)
    static constexpr COLORREF HEXRGB(DWORD rrggbb) {
        return
            ((rrggbb & 0xFF0000) >> 16) |
            ((rrggbb & 0x00FF00)) |
            ((rrggbb & 0x0000FF) << 16);
    }

    static constexpr Colors darkColors{
        HEXRGB(0x202020),   // background
        HEXRGB(0x383838),   // softerBackground
        HEXRGB(0x454545),   // hotBackground
        HEXRGB(0x202020),   // pureBackground
        HEXRGB(0xB00000),   // errorBackground
        HEXRGB(0xE0E0E0),   // textColor
        HEXRGB(0xC0C0C0),   // darkerTextColor
        HEXRGB(0x808080),   // disabledTextColor
        HEXRGB(0xFFFF00),   // linkTextColor
        HEXRGB(0x646464),   // edgeColor
        HEXRGB(0x9B9B9B),   // hotEdgeColor
        HEXRGB(0x484848)    // disabledEdgeColor
    };

    static Colors darkCustomizedColors{ darkColors };

    void initDarkMode()
    {
        const NppGUI& nppGui = NppParameters::getInstance().getNppGUI();
        _options.enable = nppGui._darkmode._isEnabled;
        _options.enablePlugin = nppGui._darkmode._isEnabledPlugin;
        g_colorToneChoice = nppGui._darkmode._colorTone;
        g_advOptions = nppGui._darkmode._advOptions;
    }

    void refreshDarkMode(HWND hwnd, bool forceRefresh)
    {
        (void)hwnd;
        (void)forceRefresh;
        // No-op on Linux - Qt handles theme changes
    }

    void initAdvancedOptions()
    {
        // Initialize from NppParameters
        const NppGUI& nppGui = NppParameters::getInstance().getNppGUI();
        g_advOptions = nppGui._darkmode._advOptions;
    }

    bool isEnabled()
    {
        return _options.enable;
    }

    bool isEnabledForPlugins()
    {
        return _options.enablePlugin;
    }

    bool isExperimentalActive()
    {
        // Linux uses Qt's native dark mode support
        return false;
    }

    bool isExperimentalSupported()
    {
        // Linux uses Qt's native dark mode support
        return false;
    }

    bool isWindowsModeEnabled()
    {
        // On Linux, this returns the stored preference
        // Actual dark mode detection is handled by Qt
        return g_advOptions._enableWindowsMode;
    }

    void setWindowsMode(bool enable)
    {
        g_advOptions._enableWindowsMode = enable;
    }

    std::wstring getThemeName()
    {
        auto& theme = NppDarkMode::isEnabled() ? g_advOptions._darkDefaults._xmlFileName : g_advOptions._lightDefaults._xmlFileName;
        return (theme == L"stylers.xml") ? L"" : theme;
    }

    void setThemeName(const std::wstring& newThemeName)
    {
        if (NppDarkMode::isEnabled())
            g_advOptions._darkDefaults._xmlFileName = newThemeName;
        else
            g_advOptions._lightDefaults._xmlFileName = newThemeName;
    }

    TbIconInfo getToolbarIconInfo(bool useDark)
    {
        auto& toolbarInfo = useDark ? g_advOptions._darkDefaults._tbIconInfo
            : g_advOptions._lightDefaults._tbIconInfo;
        return toolbarInfo;
    }

    TbIconInfo getToolbarIconInfo()
    {
        return NppDarkMode::getToolbarIconInfo(NppDarkMode::isEnabled());
    }

    void setToolbarIconSet(int state2Set, bool useDark)
    {
        if (useDark)
            g_advOptions._darkDefaults._tbIconInfo._tbIconSet = static_cast<toolBarStatusType>(state2Set);
        else
            g_advOptions._lightDefaults._tbIconInfo._tbIconSet = static_cast<toolBarStatusType>(state2Set);
    }

    void setToolbarIconSet(int state2Set)
    {
        NppDarkMode::setToolbarIconSet(state2Set, NppDarkMode::isEnabled());
    }

    void setToolbarFluentColor(FluentColor color2Set, bool useDark)
    {
        if (useDark)
            g_advOptions._darkDefaults._tbIconInfo._tbColor = color2Set;
        else
            g_advOptions._lightDefaults._tbIconInfo._tbColor = color2Set;
    }

    void setToolbarFluentColor(FluentColor color2Set)
    {
        NppDarkMode::setToolbarFluentColor(color2Set, NppDarkMode::isEnabled());
    }

    void setToolbarFluentMonochrome(bool setMonochrome, bool useDark)
    {
        if (useDark)
            g_advOptions._darkDefaults._tbIconInfo._tbUseMono = setMonochrome;
        else
            g_advOptions._lightDefaults._tbIconInfo._tbUseMono = setMonochrome;
    }

    void setToolbarFluentMonochrome(bool setMonochrome)
    {
        NppDarkMode::setToolbarFluentMonochrome(setMonochrome, NppDarkMode::isEnabled());
    }

    void setToolbarFluentCustomColor(COLORREF color, bool useDark)
    {
        if (useDark)
            g_advOptions._darkDefaults._tbIconInfo._tbCustomColor = color;
        else
            g_advOptions._lightDefaults._tbIconInfo._tbCustomColor = color;
    }

    void setToolbarFluentCustomColor(COLORREF color)
    {
        NppDarkMode::setToolbarFluentCustomColor(color, NppDarkMode::isEnabled());
    }

    int getTabIconSet(bool useDark)
    {
        return useDark ? g_advOptions._darkDefaults._tabIconSet : g_advOptions._lightDefaults._tabIconSet;
    }

    void setTabIconSet(bool useAltIcons, bool useDark)
    {
        if (useDark)
            g_advOptions._darkDefaults._tabIconSet = useAltIcons ? 1 : 2;
        else
            g_advOptions._lightDefaults._tabIconSet = useAltIcons ? 1 : 0;
    }

    bool useTabTheme()
    {
        return NppDarkMode::isEnabled() ? g_advOptions._darkDefaults._tabUseTheme : g_advOptions._lightDefaults._tabUseTheme;
    }

    void setAdvancedOptions()
    {
        // Apply advanced options to NppParameters
        NppGUI& nppGui = NppParameters::getInstance().getNppGUI();
        nppGui._darkmode._advOptions = g_advOptions;
    }

    bool isWindows10()
    {
        // Linux is not Windows 10
        return false;
    }

    bool isWindows11()
    {
        // Linux is not Windows 11
        return false;
    }

    DWORD getWindowsBuildNumber()
    {
        return 0;
    }

    COLORREF invertLightness(COLORREF c)
    {
        // Simple inversion for Linux
        return ((c & 0xFF) << 16) | (c & 0xFF00) | ((c & 0xFF0000) >> 16);
    }

    double calculatePerceivedLightness(COLORREF c)
    {
        // Calculate perceived lightness using relative luminance formula
        auto r = static_cast<double>((c >> 16) & 0xFF) / 255.0;
        auto g = static_cast<double>((c >> 8) & 0xFF) / 255.0;
        auto b = static_cast<double>(c & 0xFF) / 255.0;

        // Convert to linear RGB
        r = (r <= 0.03928) ? (r / 12.92) : pow((r + 0.055) / 1.055, 2.4);
        g = (g <= 0.03928) ? (g / 12.92) : pow((g + 0.055) / 1.055, 2.4);
        b = (b <= 0.03928) ? (b / 12.92) : pow((b + 0.055) / 1.055, 2.4);

        double luminance = 0.2126 * r + 0.7152 * g + 0.0722 * b;
        return luminance * 100.0;
    }

    void setDarkTone(ColorTone colorToneChoice)
    {
        g_colorToneChoice = colorToneChoice;
    }

    COLORREF getAccentColor(bool useDark)
    {
        (void)useDark;
        // Return a default accent color for Linux
        // Qt applications typically use the system palette
        return HEXRGB(0x0078D4); // Default blue accent
    }

    COLORREF getAccentColor()
    {
        return NppDarkMode::getAccentColor(NppDarkMode::isEnabled());
    }

    COLORREF getBackgroundColor()
    {
        return darkColors.background;
    }

    COLORREF getCtrlBackgroundColor()
    {
        return darkColors.softerBackground;
    }

    COLORREF getHotBackgroundColor()
    {
        return darkColors.hotBackground;
    }

    COLORREF getDlgBackgroundColor()
    {
        return darkColors.pureBackground;
    }

    COLORREF getErrorBackgroundColor()
    {
        return darkColors.errorBackground;
    }

    COLORREF getTextColor()
    {
        return darkColors.text;
    }

    COLORREF getDarkerTextColor()
    {
        return darkColors.darkerText;
    }

    COLORREF getDisabledTextColor()
    {
        return darkColors.disabledText;
    }

    COLORREF getLinkTextColor()
    {
        return darkColors.linkText;
    }

    COLORREF getEdgeColor()
    {
        return darkColors.edge;
    }

    COLORREF getHotEdgeColor()
    {
        return darkColors.hotEdge;
    }

    COLORREF getDisabledEdgeColor()
    {
        return darkColors.disabledEdge;
    }

    HBRUSH getBackgroundBrush()
    {
        return nullptr; // Not used on Linux
    }

    HBRUSH getDlgBackgroundBrush()
    {
        return nullptr; // Not used on Linux
    }

    HBRUSH getCtrlBackgroundBrush()
    {
        return nullptr; // Not used on Linux
    }

    HBRUSH getHotBackgroundBrush()
    {
        return nullptr; // Not used on Linux
    }

    HBRUSH getErrorBackgroundBrush()
    {
        return nullptr; // Not used on Linux
    }

    HBRUSH getEdgeBrush()
    {
        return nullptr; // Not used on Linux
    }

    HBRUSH getHotEdgeBrush()
    {
        return nullptr; // Not used on Linux
    }

    HBRUSH getDisabledEdgeBrush()
    {
        return nullptr; // Not used on Linux
    }

    HPEN getDarkerTextPen()
    {
        return nullptr; // Not used on Linux
    }

    HPEN getEdgePen()
    {
        return nullptr; // Not used on Linux
    }

    HPEN getHotEdgePen()
    {
        return nullptr; // Not used on Linux
    }

    HPEN getDisabledEdgePen()
    {
        return nullptr; // Not used on Linux
    }

    void setBackgroundColor(COLORREF c)
    {
        darkCustomizedColors.background = c;
    }

    void setCtrlBackgroundColor(COLORREF c)
    {
        darkCustomizedColors.softerBackground = c;
    }

    void setHotBackgroundColor(COLORREF c)
    {
        darkCustomizedColors.hotBackground = c;
    }

    void setDlgBackgroundColor(COLORREF c)
    {
        darkCustomizedColors.pureBackground = c;
    }

    void setErrorBackgroundColor(COLORREF c)
    {
        darkCustomizedColors.errorBackground = c;
    }

    void setTextColor(COLORREF c)
    {
        darkCustomizedColors.text = c;
    }

    void setDarkerTextColor(COLORREF c)
    {
        darkCustomizedColors.darkerText = c;
    }

    void setDisabledTextColor(COLORREF c)
    {
        darkCustomizedColors.disabledText = c;
    }

    void setLinkTextColor(COLORREF c)
    {
        darkCustomizedColors.linkText = c;
    }

    void setEdgeColor(COLORREF c)
    {
        darkCustomizedColors.edge = c;
    }

    void setHotEdgeColor(COLORREF c)
    {
        darkCustomizedColors.hotEdge = c;
    }

    void setDisabledEdgeColor(COLORREF c)
    {
        darkCustomizedColors.disabledEdge = c;
    }

    void changeCustomTheme(const Colors& colors)
    {
        darkCustomizedColors = colors;
    }

    void handleSettingChange(HWND hwnd, LPARAM lParam, bool isFromBtn)
    {
        (void)hwnd;
        (void)lParam;
        (void)isFromBtn;
        // No-op on Linux
    }

    bool isDarkModeReg()
    {
        // On Linux, check if Qt dark mode is enabled via system settings
        // For now, return the configured option
        return _options.enable;
    }

    void initExperimentalDarkMode()
    {
        // No-op on Linux - Qt handles dark mode
    }

    void setDarkMode(bool useDark, bool fixDarkScrollbar)
    {
        (void)useDark;
        (void)fixDarkScrollbar;
        // No-op on Linux - Qt handles dark mode
    }

    void allowDarkModeForApp(bool allow)
    {
        (void)allow;
        // No-op on Linux
    }

    bool allowDarkModeForWindow(HWND hWnd, bool allow)
    {
        (void)hWnd;
        (void)allow;
        return false;
    }

    void setTitleBarThemeColor(HWND hWnd)
    {
        (void)hWnd;
        // No-op on Linux - Qt handles title bars
    }

    void enableDarkScrollBarForWindowAndChildren(HWND hwnd)
    {
        (void)hwnd;
        // No-op on Linux - Qt handles scrollbars
    }

    void paintRoundRect(HDC hdc, const RECT rect, const HPEN hpen, const HBRUSH hBrush, int width, int height)
    {
        (void)hdc;
        (void)rect;
        (void)hpen;
        (void)hBrush;
        (void)width;
        (void)height;
        // No-op on Linux
    }

    void paintRoundFrameRect(HDC hdc, const RECT rect, const HPEN hpen, int width, int height)
    {
        (void)hdc;
        (void)rect;
        (void)hpen;
        (void)width;
        (void)height;
        // No-op on Linux
    }

    void subclassButtonControl(HWND hwnd)
    {
        (void)hwnd;
        // No-op on Linux
    }

    void subclassGroupboxControl(HWND hwnd)
    {
        (void)hwnd;
        // No-op on Linux
    }

    void subclassTabControl(HWND hwnd)
    {
        (void)hwnd;
        // No-op on Linux
    }

    void subclassComboBoxControl(HWND hwnd)
    {
        (void)hwnd;
        // No-op on Linux
    }

    bool subclassTabUpDownControl(HWND hwnd)
    {
        (void)hwnd;
        return false;
    }

    void subclassAndThemeButton(HWND hwnd, NppDarkModeParams p)
    {
        (void)hwnd;
        (void)p;
        // No-op on Linux
    }

    void subclassAndThemeComboBox(HWND hwnd, NppDarkModeParams p)
    {
        (void)hwnd;
        (void)p;
        // No-op on Linux
    }

    void subclassAndThemeListBoxOrEditControl(HWND hwnd, NppDarkModeParams p, bool isListBox)
    {
        (void)hwnd;
        (void)p;
        (void)isListBox;
        // No-op on Linux
    }

    void subclassAndThemeListView(HWND hwnd, NppDarkModeParams p)
    {
        (void)hwnd;
        (void)p;
        // No-op on Linux
    }

    void themeTreeView(HWND hwnd, NppDarkModeParams p)
    {
        (void)hwnd;
        (void)p;
        // No-op on Linux
    }

    void themeToolbar(HWND hwnd, NppDarkModeParams p)
    {
        (void)hwnd;
        (void)p;
        // No-op on Linux
    }

    void themeRichEdit(HWND hwnd, NppDarkModeParams p)
    {
        (void)hwnd;
        (void)p;
        // No-op on Linux
    }

    void autoSubclassAndThemeChildControls(HWND hwndParent, bool subclass, bool theme)
    {
        (void)hwndParent;
        (void)subclass;
        (void)theme;
        // No-op on Linux
    }

    void autoThemeChildControls(HWND hwndParent)
    {
        (void)hwndParent;
        // No-op on Linux
    }

    void autoSubclassAndThemePluginDockWindow(HWND hwnd)
    {
        (void)hwnd;
        // No-op on Linux
    }

    ULONG autoSubclassAndThemePlugin(HWND hwnd, ULONG dmFlags)
    {
        (void)hwnd;
        (void)dmFlags;
        return 0;
    }

    void autoSubclassCtlColor(HWND hWnd)
    {
        (void)hWnd;
        // No-op on Linux
    }

    void autoSubclassAndThemeWindowNotify(HWND hwnd)
    {
        (void)hwnd;
        // No-op on Linux
    }

    void autoSubclassWindowMenuBar(HWND hWnd)
    {
        (void)hWnd;
        // No-op on Linux
    }

    void setDarkTitleBar(HWND hwnd)
    {
        (void)hwnd;
        // No-op on Linux - Qt handles title bars
    }

    void setDarkExplorerTheme(HWND hwnd)
    {
        (void)hwnd;
        // No-op on Linux
    }

    void setDarkScrollBar(HWND hwnd)
    {
        (void)hwnd;
        // No-op on Linux - Qt handles scrollbars
    }

    void setDarkTooltips(HWND hwnd, ToolTipsType type)
    {
        (void)hwnd;
        (void)type;
        // No-op on Linux
    }

    void setDarkLineAbovePanelToolbar(HWND hwnd)
    {
        (void)hwnd;
        // No-op on Linux
    }

    void setDarkListView(HWND hwnd)
    {
        (void)hwnd;
        // No-op on Linux
    }

    void disableVisualStyle(HWND hwnd, bool doDisable)
    {
        (void)hwnd;
        (void)doDisable;
        // No-op on Linux
    }

    void calculateTreeViewStyle()
    {
        // No-op on Linux
    }

    void updateTreeViewStylePrev()
    {
        // No-op on Linux
    }

    TreeViewStyle getTreeViewStyle()
    {
        return TreeViewStyle::classic;
    }

    void setTreeViewStyle(HWND hWnd, bool force)
    {
        (void)hWnd;
        (void)force;
        // No-op on Linux
    }

    bool isThemeDark()
    {
        return _options.enable;
    }

    void setBorder(HWND hwnd, bool border)
    {
        (void)hwnd;
        (void)border;
        // No-op on Linux
    }

    void setDarkAutoCompletion()
    {
        // No-op on Linux
    }

    LRESULT onCtlColor(HDC hdc)
    {
        (void)hdc;
        return 0;
    }

    LRESULT onCtlColorCtrl(HDC hdc)
    {
        (void)hdc;
        return 0;
    }

    LRESULT onCtlColorDlg(HDC hdc)
    {
        (void)hdc;
        return 0;
    }

    LRESULT onCtlColorError(HDC hdc)
    {
        (void)hdc;
        return 0;
    }

    LRESULT onCtlColorDlgStaticText(HDC hdc, bool isTextEnabled)
    {
        (void)hdc;
        (void)isTextEnabled;
        return 0;
    }

    LRESULT onCtlColorDlgLinkText(HDC hdc, bool isTextEnabled)
    {
        (void)hdc;
        (void)isTextEnabled;
        return 0;
    }

    LRESULT onCtlColorListbox(WPARAM wParam, LPARAM lParam)
    {
        (void)wParam;
        (void)lParam;
        return 0;
    }
}
