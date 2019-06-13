#include "global.h"
#include "option_menu.h"
#include "main.h"
#include "menu.h"
#include "menu_helpers.h"
#include "scanline_effect.h"
#include "palette.h"
#include "sprite.h"
#include "task.h"
#include "bg.h"
#include "gpu_regs.h"
#include "window.h"
#include "text.h"
#include "text_window.h"
#include "international_string_util.h"
#include "string_util.h"
#include "strings.h"
#include "sound.h"
#include "constants/songs.h"
#include "new_game.h"
#include "gba/m4a_internal.h"
#include "constants/rgb.h"

extern void SetPokemonCryStereo(u32 val);

// Task data
enum
{
    TD_MENUSELECTION,
	TD_MENUSCROLL,
	TD_MENUINPUT, //blocks option menu input if true (for handling yes/no boxes)
	TD_BIKEMODE,
	TD_FULLPARTY,
	TD_BATTLESTYLE,
	TD_BATTLESCENE,
	TD_QUICKFLEE,
	TD_KEYBOARD,
	TD_FONT,
	TD_FRAMETYPE,
	TD_LOWHPSOUND,
	TD_KEYPADSOUND,
	TD_SOUNDOUTPUT,
};

// Menu items
enum
{
	MENUITEM_BIKEMODE,
	MENUITEM_FULLPARTY,
	MENUITEM_BATTLESTYLE,
	MENUITEM_BATTLESCENE,
	MENUITEM_QUICKFLEE,
	MENUITEM_KEYBOARD,
	MENUITEM_FONT,
    MENUITEM_FRAME,
	MENUITEM_LOWHPSOUND,
	MENUITEM_KEYPADSOUND,
	MENUITEM_SOUNDOUTPUT,
	MENUITEM_RESETDEFAULTS,
	MENUITEM_LOWERNUZLOCKE,
	MENUITEM_FREEZENUZLOCKE,
    MENUITEM_CANCEL,
};

#define MENUITEM_COUNT 15

// Window Ids
enum
{
	WIN_OPTIONS,
    WIN_DESCRIPTION
};

// this file's functions
static void Task_OptionMenuFadeIn(u8 taskId);
static void Task_OptionMenuProcessInput(u8 taskId);
static void Task_OptionMenuSave(u8 taskId);
static void Task_OptionMenuFadeOut(u8 taskId);
static void HighlightOptionMenuItem(u8 selection);
static void Task_CopyOptionsToTask(u8 taskId);
static u8 Task_CalcMenuShift(u8 taskId);
static void Task_PrintOptions(u8 taskId);
static void Task_DrawDescriptionWindow(u8 taskId);
static void PrepareConfirmMessage(void);
static void Task_DisplayNuzlockeConfirmMessage(u8 taskId);
static void Task_LowerNuzlocke(u8 taskId);
static void Task_DisplayFreezeConfirmMessage(u8 taskId);
static void Task_FreezeNuzlocke(u8 taskId);
static void Task_CancelConfirm(u8 taskId);
static void OnOff_DrawChoices(u8 selection, int yPos);
static u8   OnOff_ProcessInput(u8 selection, u8 taskId);
static void BikeMode_DrawChoices(u8 selection, int yPos);
static u8   BikeMode_ProcessInput(u8 selection, u8 taskId);
static void FullParty_DrawChoices(u8 selection, int yPos);
static u8   FullParty_ProcessInput(u8 selection, u8 taskId);
static void BattleStyle_DrawChoices(u8 selection, int yPos);
static u8   BattleStyle_ProcessInput(u8 selection, u8 taskId);
static void Keyboard_DrawChoices(u8 selection, int yPos);
static u8   Keyboard_ProcessInput(u8 selection, u8 taskId);
static void Font_DrawChoices(u8 selection, int yPos);
static u8   Font_ProcessInput(u8 selection, u8 taskId);
static void FrameType_DrawChoices(u8 selection, int yPos);
static u8   FrameType_ProcessInput(u8 selection, u8 taskId);
static void Sound_DrawChoices(u8 selection, int yPos);
static u8   Sound_ProcessInput(u8 selection, u8 taskId);
static void DrawOptionsBorder(void);
static void DrawDescriptionBorder(void);
static void DrawConfirmBorder(void);

// EWRAM vars
EWRAM_DATA static bool8 sArrowPressed = FALSE;

// const rom data
static const u16 sUnknown_0855C604[] = INCBIN_U16("graphics/misc/option_menu_text.gbapal");
// note: this is only used in the Japanese release
static const u8 sEqualSignGfx[] = INCBIN_U8("graphics/misc/option_menu_equals_sign.4bpp");

// Names of options (printed on left side)
static const u8 *const sOptionMenuItemsNames[MENUITEM_COUNT] =
{
    gText_BikeMode,
    gText_FullParty,
    gText_BattleStyle,
    gText_BattleScene,
    gText_QuickFlee,
    gText_Keyboard,
	gText_Font,
	gText_Frame,
	gText_LowHPSound,
	gText_KeypadSound,
	gText_SoundOutput,
	gText_ResetDefaults,
	gText_DisableNuzlockeMode,
	gText_FreezeNuzlockeMode,
    gText_OptionMenuCancel,
};

// Option descriptions, printed in bottom window
static const u8 *const sOptionsDescriptionList[MENUITEM_COUNT] = 
{
	gText_BikeModeDescription, 
	gText_FullPartyDescription, 
	gText_BattleStyleDescription, 
	gText_BattleSceneDescription, 
	gText_QuickFleeDescription, 
	gText_KeyboardDescription, 
	gText_FontDescription, 
	gText_FrameDescription, 
	gText_LowHPSoundDescription, 
	gText_KeypadSoundDescription, 
	gText_SoundOutputDescription, 
	gText_ResetDefaultsDescription, 
	gText_DisableNuzlockeModeDescription, 
	gText_FreezeNuzlockeModeDescription, 
	gText_CancelDescription
};

// Other description text
extern const u8 gText_DowngradeToHardlockeMode[];
extern const u8 gText_DowngradeToNuzlockeMode[];
extern const u8 gText_AreYouSure[];
extern const u8 gText_Nuzlocke[];
extern const u8 gText_Deadlocke[];

// Keyboard names
const u8 *gKeyboardNameList[] = 
{
	gKeyboardText_QWERTY, 
	gKeyboardText_QWERTYPlus, 
	gKeyboardText_ABC, 
	gKeyboardText_ABCPlus, 
	gKeyboardText_AZERTY, 
	gKeyboardText_AZERTYPlus, 
	gKeyboardText_Dvorak, 
	gKeyboardText_DvorakPlus, 
	gKeyboardText_Colemak, 
	gKeyboardText_ColemakPlus, 
	gKeyboardText_Vanilla
};

// Font names
const u8 *gFontNameList[] = 
{
	gFontText_Rocket, 
	gFontText_Magma, 
	gFontText_Aqua, 
	gFontText_Galactic
};

// Text window dimensions
static const struct WindowTemplate sOptionMenuWinTemplates[] =
{
	// Options window (top)
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 1,
        .width = 26,
        .height = 12,
        .paletteNum = 1,
        .baseBlock = 2
    },
	// Description window
    {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 4,
        .paletteNum = 15, // For black text
        .baseBlock = 0x13A //26 * 12 + 2 (width * height of options box + 2)
    },
};

// Dimensions for yes/no box in bottom right corner
static const struct WindowTemplate sYesNoWindowTemplate =
{
    .bg = 1,
    .tilemapLeft = 23,
    .tilemapTop = 15,
    .width = 5,
    .height = 4,
    .paletteNum = 15,
    .baseBlock = 0x1AB
};

static const struct BgTemplate sOptionMenuBgTemplates[] =
{
   {
       .bg = 0,
       .charBaseIndex = 1,
       .mapBaseIndex = 30,
       .screenSize = 0,
       .paletteMode = 0,
       .priority = 0,
       .baseTile = 0
   },
   {
       .bg = 1,
       .charBaseIndex = 1,
       .mapBaseIndex = 31,
       .screenSize = 0,
       .paletteMode = 0,
       .priority = 1,
       .baseTile = 0
   },
};

// Functions for the lower nuzlocke yes/no box
const struct YesNoFuncTable gLowerNuzlockeFuncs = {
    Task_LowerNuzlocke, Task_CancelConfirm
};

// Functions for the freeze nuzlocke yes/no box
const struct YesNoFuncTable gFreezeNuzlockeFuncs = {
    Task_FreezeNuzlocke, Task_CancelConfirm
};

// BG color (white)
static const u16 sUnknown_0855C6A0[] = {0x7FFF};

// code
static void MainCB2(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

static void VBlankCB(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

//=============================================================================================================================
//	CREATE OPTIONS MENU
//=============================================================================================================================

// Do option menu step by step
void CB2_InitOptionMenu(void)
{
    switch (gMain.state)
    {
    default:
    case 0:
        SetVBlankCallback(NULL);
        gMain.state++;
        break;
    case 1:
        DmaClearLarge16(3, (void*)(VRAM), VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        SetGpuReg(REG_OFFSET_DISPCNT, 0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sOptionMenuBgTemplates, ARRAY_COUNT(sOptionMenuBgTemplates));
        ChangeBgX(0, 0, 0);
        ChangeBgY(0, 0, 0);
        ChangeBgX(1, 0, 0);
        ChangeBgY(1, 0, 0);
        ChangeBgX(2, 0, 0);
        ChangeBgY(2, 0, 0);
        ChangeBgX(3, 0, 0);
        ChangeBgY(3, 0, 0);
        InitWindows(sOptionMenuWinTemplates);
        DeactivateAllTextPrinters();
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WININ, 1);
        SetGpuReg(REG_OFFSET_WINOUT, 35);
        SetGpuReg(REG_OFFSET_BLDCNT, 193);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 4);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
        ShowBg(0);
        ShowBg(1);
        gMain.state++;
        break;
    case 2:
        ResetPaletteFade();
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        gMain.state++;
        break;
    case 3:
        LoadBgTiles(1, GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->tiles, 0x120, 0x1A2);
        gMain.state++;
        break;
    case 4:
        LoadPalette(sUnknown_0855C6A0, 0, sizeof(sUnknown_0855C6A0));
        LoadPalette(GetWindowFrameTilesPal(gSaveBlock2Ptr->optionsWindowFrameType)->pal, 0x70, 0x20);
        gMain.state++;
        break;
    case 5:
        LoadPalette(sUnknown_0855C604, 0x10, sizeof(sUnknown_0855C604));
        gMain.state++;
        break;
    case 6:
        PutWindowTilemap(0);
        gMain.state++;
        break;
    case 7:
        gMain.state++;
        break;
    case 8:
        PutWindowTilemap(1);
        gMain.state++;
    case 9:
		DrawOptionsBorder();
		DrawDescriptionBorder();
        gMain.state++;
        break;
    case 10:
    {
        u8 taskId = CreateTask(Task_OptionMenuFadeIn, 0);
		u16 palette;
		gTasks[taskId].data[TD_MENUSELECTION] = 0;
		gTasks[taskId].data[TD_MENUSCROLL] = 0;
		// Reset color 2 in palette 15 to white to draw option menu correctly (is overwritten in main_menu.c)
		// Without this, the background of the description window will be borked
		palette = RGB(31, 31, 31);
        LoadPalette(&palette, 241, 2);
		Task_DrawDescriptionWindow(taskId);
		HighlightOptionMenuItem(gTasks[taskId].data[TD_MENUSELECTION]);
        Task_CopyOptionsToTask(taskId);
		Task_PrintOptions(taskId);

        gMain.state++;
        break;
    }
    case 11:
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0x10, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB);
        SetMainCallback2(MainCB2);
        return;
    }
}

static void Task_OptionMenuFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_OptionMenuProcessInput;
}

static void Task_OptionMenuProcessInput(u8 taskId)
{
	if (gTasks[taskId].data[TD_MENUINPUT])
		return;
	
    if (gMain.newKeys & A_BUTTON)
    {
		switch (gTasks[taskId].data[TD_MENUSELECTION] + gTasks[taskId].data[TD_MENUSCROLL] + Task_CalcMenuShift(taskId))
		{
			case MENUITEM_RESETDEFAULTS:
				PlaySE(SE_SELECT);	
				SetDefaultOptions();
				Task_CopyOptionsToTask(taskId);
				LoadBgTiles(1, GetWindowFrameTilesPal(gTasks[taskId].data[TD_FRAMETYPE])->tiles, 0x120, 0x1A2);
				LoadPalette(GetWindowFrameTilesPal(gTasks[taskId].data[TD_FRAMETYPE])->pal, 0x70, 0x20);
				Task_PrintOptions(taskId);
				break;
			case MENUITEM_LOWERNUZLOCKE:
				PlaySE(SE_SELECT);	
				gTasks[taskId].data[TD_MENUINPUT] = TRUE;
				Task_DisplayNuzlockeConfirmMessage(taskId);
				break;
			case MENUITEM_FREEZENUZLOCKE:
				PlaySE(SE_SELECT);	
				gTasks[taskId].data[TD_MENUINPUT] = TRUE;
				Task_DisplayFreezeConfirmMessage(taskId);
				break;
			case MENUITEM_CANCEL:
				PlaySE(SE_SELECT);
				gTasks[taskId].func = Task_OptionMenuSave;
				break;
		}
    }
    else if (gMain.newKeys & B_BUTTON)
    {
        gTasks[taskId].func = Task_OptionMenuSave;
    }
    else if (gMain.newKeys & DPAD_UP)
    {
        if (gTasks[taskId].data[TD_MENUSELECTION] > 0)
            gTasks[taskId].data[TD_MENUSELECTION]--;
		else if (gTasks[taskId].data[TD_MENUSELECTION] + gTasks[taskId].data[TD_MENUSCROLL] != MENUITEM_BIKEMODE)
		{
            gTasks[taskId].data[TD_MENUSCROLL]--;
			Task_PrintOptions(taskId);
		}
        HighlightOptionMenuItem(gTasks[taskId].data[TD_MENUSELECTION]);
		Task_DrawDescriptionWindow(taskId);
    }
    else if (gMain.newKeys & DPAD_DOWN)
    {
        if (gTasks[taskId].data[TD_MENUSELECTION] < 5)
            gTasks[taskId].data[TD_MENUSELECTION]++;
		else if (gTasks[taskId].data[TD_MENUSELECTION] + gTasks[taskId].data[TD_MENUSCROLL] + Task_CalcMenuShift(taskId) != MENUITEM_CANCEL)
		{
            gTasks[taskId].data[TD_MENUSCROLL]++;
			Task_PrintOptions(taskId);
		}
        HighlightOptionMenuItem(gTasks[taskId].data[TD_MENUSELECTION]);
		Task_DrawDescriptionWindow(taskId);
    }
    else
    {
        u8 previousOption;

        switch (gTasks[taskId].data[TD_MENUSELECTION] + gTasks[taskId].data[TD_MENUSCROLL])
        {
        case MENUITEM_BIKEMODE:
            previousOption = gTasks[taskId].data[TD_BIKEMODE];
            gTasks[taskId].data[TD_BIKEMODE] = BikeMode_ProcessInput(gTasks[taskId].data[TD_BIKEMODE], taskId);

            if (previousOption != gTasks[taskId].data[TD_BIKEMODE])
                BikeMode_DrawChoices(gTasks[taskId].data[TD_BIKEMODE], gTasks[taskId].data[TD_MENUSELECTION] * 16 - 8);
            break;
		case MENUITEM_FULLPARTY:
            previousOption = gTasks[taskId].data[TD_FULLPARTY];
            gTasks[taskId].data[TD_FULLPARTY] = FullParty_ProcessInput(gTasks[taskId].data[TD_FULLPARTY], taskId);

            if (previousOption != gTasks[taskId].data[TD_FULLPARTY])
                FullParty_DrawChoices(gTasks[taskId].data[TD_FULLPARTY], gTasks[taskId].data[TD_MENUSELECTION] * 16 - 8);
            break;
		case MENUITEM_BATTLESTYLE:
            previousOption = gTasks[taskId].data[TD_BATTLESTYLE];
            gTasks[taskId].data[TD_BATTLESTYLE] = BattleStyle_ProcessInput(gTasks[taskId].data[TD_BATTLESTYLE], taskId);

            if (previousOption != gTasks[taskId].data[TD_BATTLESTYLE])
                BattleStyle_DrawChoices(gTasks[taskId].data[TD_BATTLESTYLE], gTasks[taskId].data[TD_MENUSELECTION] * 16 - 8);
            break;
		case MENUITEM_BATTLESCENE:
            previousOption = gTasks[taskId].data[TD_BATTLESCENE];
            gTasks[taskId].data[TD_BATTLESCENE] = OnOff_ProcessInput(gTasks[taskId].data[TD_BATTLESCENE], taskId);

            if (previousOption != gTasks[taskId].data[TD_BATTLESCENE])
                OnOff_DrawChoices(gTasks[taskId].data[TD_BATTLESCENE], gTasks[taskId].data[TD_MENUSELECTION] * 16 - 8);
            break;
		case MENUITEM_QUICKFLEE:
            previousOption = gTasks[taskId].data[TD_QUICKFLEE];
            gTasks[taskId].data[TD_QUICKFLEE] = OnOff_ProcessInput(gTasks[taskId].data[TD_QUICKFLEE], taskId);

            if (previousOption != gTasks[taskId].data[TD_QUICKFLEE])
                OnOff_DrawChoices(gTasks[taskId].data[TD_QUICKFLEE], gTasks[taskId].data[TD_MENUSELECTION] * 16 - 8);
            break;
		case MENUITEM_KEYBOARD:
            previousOption = gTasks[taskId].data[TD_KEYBOARD];
            gTasks[taskId].data[TD_KEYBOARD] = Keyboard_ProcessInput(gTasks[taskId].data[TD_KEYBOARD], taskId);

            if (previousOption != gTasks[taskId].data[TD_KEYBOARD])
			{
                Keyboard_DrawChoices(gTasks[taskId].data[TD_KEYBOARD], gTasks[taskId].data[TD_MENUSELECTION] * 16 - 8);
				Task_PrintOptions(taskId);
			}
            break;
		case MENUITEM_FONT:
            previousOption = gTasks[taskId].data[TD_FONT];
            gTasks[taskId].data[TD_FONT] = Font_ProcessInput(gTasks[taskId].data[TD_FONT], taskId);

            if (previousOption != gTasks[taskId].data[TD_FONT])
			{
                Font_DrawChoices(gTasks[taskId].data[TD_FONT], gTasks[taskId].data[TD_MENUSELECTION] * 16 - 8);
				Task_PrintOptions(taskId);
			}
            break;
		case MENUITEM_FRAME:
            previousOption = gTasks[taskId].data[TD_FRAMETYPE];
            gTasks[taskId].data[TD_FRAMETYPE] = FrameType_ProcessInput(gTasks[taskId].data[TD_FRAMETYPE], taskId);

            if (previousOption != gTasks[taskId].data[TD_FRAMETYPE])
			{
                FrameType_DrawChoices(gTasks[taskId].data[TD_FRAMETYPE], gTasks[taskId].data[TD_MENUSELECTION] * 16);
				Task_PrintOptions(taskId);
			}
            break;
		case MENUITEM_LOWHPSOUND:
            previousOption = gTasks[taskId].data[TD_LOWHPSOUND];
            gTasks[taskId].data[TD_LOWHPSOUND] = OnOff_ProcessInput(gTasks[taskId].data[TD_LOWHPSOUND], taskId);

            if (previousOption != gTasks[taskId].data[TD_LOWHPSOUND])
                OnOff_DrawChoices(gTasks[taskId].data[TD_LOWHPSOUND], gTasks[taskId].data[TD_MENUSELECTION] * 16 - 8);
            break;
		case MENUITEM_KEYPADSOUND:
            previousOption = gTasks[taskId].data[TD_KEYPADSOUND];
            gTasks[taskId].data[TD_KEYPADSOUND] = OnOff_ProcessInput(gTasks[taskId].data[TD_KEYPADSOUND], taskId);

            if (previousOption != gTasks[taskId].data[TD_KEYPADSOUND])
                OnOff_DrawChoices(gTasks[taskId].data[TD_KEYPADSOUND], gTasks[taskId].data[TD_MENUSELECTION] * 16 - 8);
            break;
		case MENUITEM_SOUNDOUTPUT:
            previousOption = gTasks[taskId].data[TD_SOUNDOUTPUT];
            gTasks[taskId].data[TD_SOUNDOUTPUT] = Sound_ProcessInput(gTasks[taskId].data[TD_SOUNDOUTPUT], taskId);

            if (previousOption != gTasks[taskId].data[TD_SOUNDOUTPUT])
                Sound_DrawChoices(gTasks[taskId].data[TD_SOUNDOUTPUT], gTasks[taskId].data[TD_MENUSELECTION] * 16 - 8);
            break;
        default:
            return;
        }
		
        if (sArrowPressed)
        {
            sArrowPressed = FALSE;
            CopyWindowToVram(WIN_OPTIONS, 2);
        }
    }
}

static void Task_OptionMenuSave(u8 taskId)
{
    gSaveBlock2Ptr->optionsBikeMode = gTasks[taskId].data[TD_BIKEMODE];
	gSaveBlock2Ptr->optionsFullParty = gTasks[taskId].data[TD_FULLPARTY];
	gSaveBlock2Ptr->optionsBattleStyle = gTasks[taskId].data[TD_BATTLESTYLE];
	gSaveBlock2Ptr->optionsBattleSceneOff = gTasks[taskId].data[TD_BATTLESCENE];
	gSaveBlock2Ptr->optionsQuickFlee = gTasks[taskId].data[TD_QUICKFLEE];
	gSaveBlock2Ptr->optionsKeyboard = gTasks[taskId].data[TD_KEYBOARD];
	gSaveBlock2Ptr->optionsFont = gTasks[taskId].data[TD_FONT];
	gSaveBlock2Ptr->optionsWindowFrameType = gTasks[taskId].data[TD_FRAMETYPE];
	gSaveBlock2Ptr->optionsLowHPSound = gTasks[taskId].data[TD_LOWHPSOUND];
	gSaveBlock2Ptr->optionsKeypadSound = gTasks[taskId].data[TD_KEYPADSOUND];
	gSaveBlock2Ptr->optionsSound = gTasks[taskId].data[TD_SOUNDOUTPUT];

    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 0x10, RGB_BLACK);
    gTasks[taskId].func = Task_OptionMenuFadeOut;
}

static void Task_OptionMenuFadeOut(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(gMain.savedCallback);
    }
}

static void HighlightOptionMenuItem(u8 index)
{
    SetGpuReg(REG_OFFSET_WIN0H, WIN_RANGE(16, 224));
    SetGpuReg(REG_OFFSET_WIN0V, WIN_RANGE(index * 16 + 8, index * 16 + 24));
}

static void Task_CopyOptionsToTask(u8 taskId)
{
	gTasks[taskId].data[TD_BIKEMODE] = gSaveBlock2Ptr->optionsBikeMode;
	gTasks[taskId].data[TD_FULLPARTY] = gSaveBlock2Ptr->optionsFullParty;
	gTasks[taskId].data[TD_BATTLESTYLE] = gSaveBlock2Ptr->optionsBattleStyle;
	gTasks[taskId].data[TD_BATTLESCENE] = gSaveBlock2Ptr->optionsBattleSceneOff;
	gTasks[taskId].data[TD_QUICKFLEE] = gSaveBlock2Ptr->optionsQuickFlee;
	gTasks[taskId].data[TD_KEYBOARD] = gSaveBlock2Ptr->optionsKeyboard;
	gTasks[taskId].data[TD_FONT] = gSaveBlock2Ptr->optionsFont;
	gTasks[taskId].data[TD_FRAMETYPE] = gSaveBlock2Ptr->optionsWindowFrameType;
	gTasks[taskId].data[TD_LOWHPSOUND] = gSaveBlock2Ptr->optionsLowHPSound;
	gTasks[taskId].data[TD_KEYPADSOUND] = gSaveBlock2Ptr->optionsKeypadSound;
	gTasks[taskId].data[TD_SOUNDOUTPUT] = gSaveBlock2Ptr->optionsSound;
}

//=============================================================================================================================
//	PRINT STRINGS AND DESCRIPTIONS
//=============================================================================================================================

// Compensates for missing nuzlocke options when they aren't present
static u8 Task_CalcMenuShift(u8 taskId)
{
	if (gTasks[taskId].data[TD_MENUSELECTION] + gTasks[taskId].data[TD_MENUSCROLL] > MENUITEM_RESETDEFAULTS)
	{
		if (gSaveBlock2Ptr->nuzlockeMode == NUZLOCKE_MODE_OFF
			|| gSaveBlock2Ptr->freezeNuzlocke == TRUE)
			return 2; //no nuzlocke options
		else
			return 0;
	}
	else
		return 0;
}

// Creates the list of options in the top window
static void Task_PrintOptions(u8 taskId)
{
	int i;
	
	FillWindowPixelBuffer(WIN_OPTIONS, 0x11); // Clear window first
	
	for (i = 0; i < 6; i++) // 6 options on screen at once
	{
		// Print "close menu" instead of nuzlocke options when scrolling past "reset default options" if Nuzlocke is off or frozen
		if (gTasks[taskId].data[TD_MENUSCROLL] + i > MENUITEM_RESETDEFAULTS
			&& (gSaveBlock2Ptr->nuzlockeMode == NUZLOCKE_MODE_OFF
			|| gSaveBlock2Ptr->freezeNuzlocke == TRUE))
		{
			AddTextPrinterParameterized(WIN_OPTIONS, 1, gText_OptionMenuCancel, 8, (i * 16) + 1, TEXT_SPEED_FF, NULL);
		}
		// If nuzlocke is on or unfrozen and that option will be printed, get the correct string
		else if (sOptionMenuItemsNames[gTasks[taskId].data[TD_MENUSCROLL] + i] == gText_DisableNuzlockeMode)
		{
			switch (gSaveBlock2Ptr->nuzlockeMode)
			{
				case NUZLOCKE_MODE_DEADLOCKE:
					AddTextPrinterParameterized(WIN_OPTIONS, 1, gText_DowngradeToHardlockeMode, 8, (i * 16) + 1, TEXT_SPEED_FF, NULL);
					break;
				case NUZLOCKE_MODE_HARDLOCKE:
					AddTextPrinterParameterized(WIN_OPTIONS, 1, gText_DowngradeToNuzlockeMode, 8, (i * 16) + 1, TEXT_SPEED_FF, NULL);
					break;
				case NUZLOCKE_MODE_NUZLOCKE:
					AddTextPrinterParameterized(WIN_OPTIONS, 1, gText_DisableNuzlockeMode, 8, (i * 16) + 1, TEXT_SPEED_FF, NULL);
					break;
			}
		}
		// Otherwise, just print the relevant option name
		else
			AddTextPrinterParameterized(WIN_OPTIONS, 1, sOptionMenuItemsNames[i + gTasks[taskId].data[TD_MENUSCROLL]], 8, (i * 16) + 1, TEXT_SPEED_FF, NULL);
		
		// Draw choices for each option
		switch (i + gTasks[taskId].data[TD_MENUSCROLL])
		{
			case 0: //BIKE MODE
				BikeMode_DrawChoices(gTasks[taskId].data[TD_BIKEMODE], i * 16 - 8);
				break;
				
			case 1: //FULL PARTY
				FullParty_DrawChoices(gTasks[taskId].data[TD_FULLPARTY], i * 16 - 8);
				break;
				
			case 2: //BATTLE STYLE
				BattleStyle_DrawChoices(gTasks[taskId].data[TD_BATTLESTYLE], i * 16 - 8);
				break;
				
			case 3: //BATTLE SCENE
				OnOff_DrawChoices(gTasks[taskId].data[TD_BATTLESCENE], i * 16 - 8);
				break;
				
			case 4: //QUICK FLEE
				OnOff_DrawChoices(gTasks[taskId].data[TD_QUICKFLEE], i * 16 - 8);
				break;	
				
			case 5: //KEYBOARD
				Keyboard_DrawChoices(gTasks[taskId].data[TD_KEYBOARD], i * 16 - 8);
				break;
				
			case 6: //FONT
				Font_DrawChoices(gTasks[taskId].data[TD_FONT], i * 16 - 8);
				break;
				
			case 7: //FRAME
				FrameType_DrawChoices(gTasks[taskId].data[TD_FRAMETYPE], i * 16);
				break;
				
			case 8: //LOW HP SOUND
				OnOff_DrawChoices(gTasks[taskId].data[TD_LOWHPSOUND], i * 16 - 8);
				break;
				
			case 9: //KEYPAD SOUND
				OnOff_DrawChoices(gTasks[taskId].data[TD_KEYPADSOUND], i * 16 - 8);
				break;
				
			case 10: //SOUND OUTPUT
				Sound_DrawChoices(gTasks[taskId].data[TD_SOUNDOUTPUT], i * 16 - 8);
				break;
		}
	}
	
	// Copy the whole thing to the window
	CopyWindowToVram(WIN_OPTIONS, 3);
}

static void Task_DrawDescriptionWindow(u8 taskId)
{
    u8 i;

	FillWindowPixelBuffer(WIN_DESCRIPTION, 0x11);
	if (sOptionsDescriptionList[gTasks[taskId].data[TD_MENUSELECTION] + gTasks[taskId].data[TD_MENUSCROLL] + Task_CalcMenuShift(taskId)] == gText_DisableNuzlockeModeDescription)
	{
		switch (gSaveBlock2Ptr->nuzlockeMode)
		{
			case NUZLOCKE_MODE_DEADLOCKE:
				AddTextPrinterParameterized(WIN_DESCRIPTION, 1, gText_DowngradeToHardlockeModeDescription, 8, 1, TEXT_SPEED_FF, NULL);
				break;
			case NUZLOCKE_MODE_HARDLOCKE:
				AddTextPrinterParameterized(WIN_DESCRIPTION, 1, gText_DowngradeToNuzlockeModeDescription, 8, 1, TEXT_SPEED_FF, NULL);
				break;
			case NUZLOCKE_MODE_NUZLOCKE:
				AddTextPrinterParameterized(WIN_DESCRIPTION, 1, gText_DisableNuzlockeModeDescription, 8, 1, TEXT_SPEED_FF, NULL);
				break;
		}
	}
	else
		AddTextPrinterParameterized(WIN_DESCRIPTION, 1, sOptionsDescriptionList[gTasks[taskId].data[TD_MENUSELECTION] + gTasks[taskId].data[TD_MENUSCROLL] + Task_CalcMenuShift(taskId)], 8, 1, TEXT_SPEED_FF, NULL);
    CopyWindowToVram(WIN_DESCRIPTION, 3);
}

// Does everything to display a yes/no confirm box
static void PrepareConfirmMessage()
{
	// Creates confirm window border
	DrawConfirmBorder();
	// Clear window
	FillWindowPixelBuffer(WIN_DESCRIPTION, 0x11);
	// Print confirmation text
	AddTextPrinterParameterized(WIN_DESCRIPTION, 1, gText_AreYouSure, 8, 1, TEXT_SPEED_FF, NULL);
    CopyWindowToVram(WIN_DESCRIPTION, 3);
	// Add yes/no box
	sub_81973A4();
	CreateYesNoMenu(&sYesNoWindowTemplate, 0x214, 14, 1);
}

// Lower nuzlocke option was selected
static void Task_DisplayNuzlockeConfirmMessage(u8 taskId)
{
	PrepareConfirmMessage();
	DoYesNoFuncWithChoice(taskId, &gLowerNuzlockeFuncs);
}

// Player has chosen to lower nuzlocke level from Task_DisplayNuzlockeConfirmMessage
static void Task_LowerNuzlocke(u8 taskId)
{
	// If nuzlocke isn't already off, lower by 1 stage
	if (gSaveBlock2Ptr->nuzlockeMode != NUZLOCKE_MODE_OFF)
		gSaveBlock2Ptr->nuzlockeMode--;
	// If it is now off, adjust menu scrolling
	if (gSaveBlock2Ptr->nuzlockeMode == NUZLOCKE_MODE_OFF)
		gTasks[taskId].data[TD_MENUSCROLL] -= 2;
	// Return to option menu
	Task_CancelConfirm(taskId);
}

// Freeze nuzlocke option was selected
static void Task_DisplayFreezeConfirmMessage(u8 taskId)
{
	PrepareConfirmMessage();
	DoYesNoFuncWithChoice(taskId, &gFreezeNuzlockeFuncs);
}

// Player has chosen to lower nuzlocke level from Task_DisplayNuzlockeConfirmMessage
static void Task_FreezeNuzlocke(u8 taskId)
{
	// Set freeze nuzlocke to TRUE
	gSaveBlock2Ptr->freezeNuzlocke = TRUE;
	// Adjust menu scrolling
	gTasks[taskId].data[TD_MENUSCROLL] -= 2;
	// Return to option menu
	Task_CancelConfirm(taskId);
}

// Player has chosen not to lower nuzlocke level, freeze nuzlocke mode
// Or, they HAVE chosen to in which case this will run after Task_LowerNuzlocke or Task_FreezeNuzlocke
static void Task_CancelConfirm(u8 taskId)
{
	// Redraw option description and border
	PutWindowTilemap(1); // Clears the extra piece of border from the confirm window
	Task_DrawDescriptionWindow(taskId);
	DrawDescriptionBorder();
	// Update options
	Task_PrintOptions(taskId);
	// Re-enable input into the main options menu and return
	gTasks[taskId].data[TD_MENUINPUT] = FALSE;
	gTasks[taskId].func = Task_OptionMenuProcessInput;
}

static void DrawOptionMenuChoice(const u8 *text, u8 x, u8 y, u8 style)
{
    u8 dst[16];
    u16 i;

    for (i = 0; *text != EOS && i <= 20; i++)
        dst[i] = *(text++);

    if (style != 0)
    {
        dst[2] = 4;
        dst[5] = 5;
    }

    dst[i] = EOS;
    AddTextPrinterParameterized(WIN_OPTIONS, 1, dst, x, y + 1, TEXT_SPEED_FF, NULL);
}

//=============================================================================================================================
//	DRAW MENU OPTIONS & PROCESS INPUT
//=============================================================================================================================

static void OnOff_DrawChoices(u8 selection, int yPos)
{
    u8 styles[2];

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_On,  106, 8 + yPos, styles[0]);
    DrawOptionMenuChoice(gText_Off, 176, 8 + yPos, styles[1]);
}

static u8 OnOff_ProcessInput(u8 selection, u8 taskId)
{
    if (gMain.newKeys & (DPAD_LEFT | DPAD_RIGHT))
	{
        selection ^= 1;
		sArrowPressed = TRUE;
	}
    return selection;
}

static u8 BikeMode_ProcessInput(u8 selection, u8 taskId)
{
    if (gMain.newKeys & (DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
		sArrowPressed = TRUE;
    }
    return selection;
}

static void BikeMode_DrawChoices(u8 selection, int yPos)
{
    u8 styles[3];

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_HoldB, 106, 8 + yPos, styles[0]);
    DrawOptionMenuChoice(gText_Auto,  170, 8 + yPos, styles[1]);
}

static void FullParty_DrawChoices(u8 selection, int yPos)
{
    u8 styles[2];
	
    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_SwapPC, 106, 8 + yPos, styles[0]);
    DrawOptionMenuChoice(gText_SendToPC, 140, 8 + yPos, styles[1]);
}

static u8 FullParty_ProcessInput(u8 selection, u8 taskId)
{
	if (gMain.newKeys & (DPAD_LEFT | DPAD_RIGHT))
	{
		// Change option as normal if not in Deadlocke mode
		if (gSaveBlock2Ptr->nuzlockeMode != NUZLOCKE_MODE_DEADLOCKE)
		{
			selection ^= 1;
			sArrowPressed = TRUE;
			return selection;
		}
		// Play error sound effect, display a message and don't change option if in Deadlocke mode
		else
		{
			FillWindowPixelBuffer(WIN_DESCRIPTION, 0x11);
			StringCopy(gStringVar1, gText_Deadlocke);
			StringExpandPlaceholders(gStringVar4, gText_CantBeChanged);
			AddTextPrinterParameterized(WIN_DESCRIPTION, 1, gStringVar4, 8, 1, TEXT_SPEED_FF, NULL);
			CopyWindowToVram(WIN_DESCRIPTION, 3);
			PlaySE(SE_BOO);
			// Return SWAP
			return 0;
		}
	}
}

static void BattleStyle_DrawChoices(u8 selection, int yPos)
{
    u8 styles[2];

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_BattleStyleShift, 106, 8 + yPos, styles[0]);
    DrawOptionMenuChoice(gText_BattleStyleSet,   176,   8 + yPos, styles[1]);
}

static u8 BattleStyle_ProcessInput(u8 selection, u8 taskId)
{
	if (gMain.newKeys & (DPAD_LEFT | DPAD_RIGHT))
	{
		// Change option as normal if not in Nuzlocke mode
		if (gSaveBlock2Ptr->nuzlockeMode == NUZLOCKE_MODE_OFF)
		{
			selection ^= 1;
			sArrowPressed = TRUE;
			return selection;
		}
		// Play error sound effect, display a message and don't change option if in Nuzlocke mode
		else
		{
			FillWindowPixelBuffer(WIN_DESCRIPTION, 0x11);
			StringCopy(gStringVar1, gText_Nuzlocke);
			StringExpandPlaceholders(gStringVar4, gText_CantBeChanged);
			AddTextPrinterParameterized(WIN_DESCRIPTION, 1, gStringVar4, 8, 1, TEXT_SPEED_FF, NULL);
			CopyWindowToVram(WIN_DESCRIPTION, 3);
			PlaySE(SE_BOO);
			// Return SET
			return 1;
		}
	}
}

static void Keyboard_DrawChoices(u8 selection, int yPos)
{
    DrawOptionMenuChoice(gKeyboardNameList[selection], 106, 8 + yPos, 1);
}

static u8 Keyboard_ProcessInput(u8 selection, u8 taskId)
{
	if ((gMain.newKeys & DPAD_RIGHT) || (gMain.newKeys & DPAD_LEFT))
	{
		if (gMain.newKeys & DPAD_RIGHT)
		{
			if (selection < 10)
				selection++;
			else
				selection = 0;
		}
		if (gMain.newKeys & DPAD_LEFT)
		{
			if (selection > 0)
				selection--;
			else
				selection = 10;
		}
		sArrowPressed = TRUE;
	}
    return selection;
}

static void Font_DrawChoices(u8 selection, int yPos)
{
	DrawOptionMenuChoice(gFontNameList[selection], 106, 8 + yPos, 1);
}

static u8 Font_ProcessInput(u8 selection, u8 taskId)
{
	if ((gMain.newKeys & DPAD_RIGHT) || (gMain.newKeys & DPAD_LEFT))
	{
		if (gMain.newKeys & DPAD_RIGHT)
		{
			if (selection < 3) //4 types of keyboard
				selection++;
			else
				selection = 0;
		}
		if (gMain.newKeys & DPAD_LEFT)
		{
			if (selection > 0)
				selection--;
			else
				selection = 3;
		}
		sArrowPressed = TRUE;
	}
    return selection;
}

#define CHAR_0 0xA1 //Character code of '0' character
#define CHAR_LEFT_ARROW 0x7B //Character code of right arrow character
#define CHAR_RIGHT_ARROW 0x7C //Character code of right arrow character

static void FrameType_DrawChoices(u8 selection, int yPos)
{
    u8 text[8];
    u8 n = selection + 1;
    u16 i;

    for (i = 0; gText_FrameTypeNumber[i] != EOS && i < 6; i++)
        text[i] = gText_FrameTypeNumber[i];
	
	text[i] = CHAR_LEFT_ARROW;
	i++;
	text[i] = CHAR_SPACE;
	i++;

    //Convert number to decimal string
    if (n / 10 != 0)
    {
        text[i] = n / 10 + CHAR_0;
        i++;
        text[i] = n % 10 + CHAR_0;
        i++;
		text[i] = CHAR_SPACE;
        i++;
    }
    else
    {
        text[i] = n % 10 + CHAR_0;
        i++;
        text[i] = CHAR_SPACE;
        i++;
    }
	
	text[i] = CHAR_RIGHT_ARROW;
	i++;
    text[i] = EOS;

    DrawOptionMenuChoice(gText_FrameType, 106, yPos, 0);
    DrawOptionMenuChoice(text, 139, yPos, 1);
}

static u8 FrameType_ProcessInput(u8 selection, u8 taskId)
{
	if ((gMain.newKeys & DPAD_RIGHT) || (gMain.newKeys & DPAD_LEFT))
	{
		if (gMain.newKeys & DPAD_RIGHT)
		{
			if (selection < WINDOW_FRAMES_COUNT - 1)
				selection++;
			else
				selection = 0;
			
			LoadBgTiles(1, GetWindowFrameTilesPal(selection)->tiles, 0x120, 0x1A2);
			LoadPalette(GetWindowFrameTilesPal(selection)->pal, 0x70, 0x20);
			sArrowPressed = TRUE;
		}
		if (gMain.newKeys & DPAD_LEFT)
		{
			if (selection > 0)
				selection--;
			else
				selection = WINDOW_FRAMES_COUNT - 1;
			
			LoadBgTiles(1, GetWindowFrameTilesPal(selection)->tiles, 0x120, 0x1A2);
			LoadPalette(GetWindowFrameTilesPal(selection)->pal, 0x70, 0x20);
			sArrowPressed = TRUE;
		}
		sArrowPressed = TRUE;
	}
    return selection;
}

static void Sound_DrawChoices(u8 selection, int yPos)
{
    u8 styles[2];

    styles[0] = 0;
    styles[1] = 0;
    styles[selection] = 1;

    DrawOptionMenuChoice(gText_SoundMono,   106, 8 + yPos, styles[0]);
    DrawOptionMenuChoice(gText_SoundStereo, 158, 8 + yPos, styles[1]);
}

static u8 Sound_ProcessInput(u8 selection, u8 taskId)
{
    if (gMain.newKeys & (DPAD_LEFT | DPAD_RIGHT))
    {
        selection ^= 1;
		sArrowPressed = TRUE;
        SetPokemonCryStereo(selection);
    }
    return selection;
}

//=============================================================================================================================
//	DRAW WINDOW BORDERS
//=============================================================================================================================

// Creates the frame of the top options window
static void DrawOptionsBorder(void)
{
    //                   bg, tileNum, x,    y,    width, height,  pal
	FillBgTilemapBufferRect(1, 0x1A2, 1,    0,      1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A3, 2,    0,      0x1B,   1,      7);
    FillBgTilemapBufferRect(1, 0x1A4, 28,   0,      1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A5, 1,    1,      1,      12,     7);
    FillBgTilemapBufferRect(1, 0x1A7, 28,   1,      1,      12,     7);
    FillBgTilemapBufferRect(1, 0x1A8, 1,    13,     1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A9, 2,    13,     0x1B,   1,      7);
    FillBgTilemapBufferRect(1, 0x1AA, 28,   13,     1,      1,      7);
	CopyBgTilemapBufferToVram(1);
}

// Creates the frame of the bottom window that contains the option descriptions
static void DrawDescriptionBorder(void)
{
	//                   bg, tileNum, x,    y,    width, height,  pal
    FillBgTilemapBufferRect(1, 0x1A2, 1,    14,     1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A3, 2,    14,     0x1A,   1,      7);
    FillBgTilemapBufferRect(1, 0x1A4, 28,   14,     1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A5, 1,    15,     1,      4,      7);
    FillBgTilemapBufferRect(1, 0x1A7, 28,   15,     1,      4,      7);
    FillBgTilemapBufferRect(1, 0x1A8, 1,    19,     1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A9, 2,    19,     0x1A,   1,      7);
    FillBgTilemapBufferRect(1, 0x1AA, 28,   19,     1,      1,      7);
    //FillBgTilemapBufferRect(1, 0x0, 21,   15,     1,      4,      7);
	CopyBgTilemapBufferToVram(1);
}

// Creates the "are you sure?" message frame when using Nuzlocke related options
static void DrawConfirmBorder(void)
{
    //                   bg, tileNum, x,    y,    width, height,  pal
	FillBgTilemapBufferRect(1, 0x1A2, 1,    14,     1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A3, 2,    14,     0x13,   1,      7);
    FillBgTilemapBufferRect(1, 0x1A4, 21,   14,     1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A5, 1,    15,     1,      4,      7);
    FillBgTilemapBufferRect(1, 0x1A7, 21,   15,     1,      4,      7);
    FillBgTilemapBufferRect(1, 0x1A8, 1,    19,     1,      1,      7);
    FillBgTilemapBufferRect(1, 0x1A9, 2,    19,     0x13,   1,      7);
    FillBgTilemapBufferRect(1, 0x1AA, 21,   19,     1,      1,      7);
	CopyBgTilemapBufferToVram(1);
}
