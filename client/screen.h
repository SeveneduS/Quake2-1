void	SCR_Init (void);
void	SCR_UpdateScreen (void);

void	SCR_CenterPrint (char *str);

//void	SCR_BeginLoadingPlaque (void); -- declared in qcommon.h
void	SCR_EndLoadingPlaque (bool force);	//?? is "force" needed ?

void	SCR_DebugGraph (float value, int color);

void	SCR_TouchPics (void);

// here for menu:
extern	cvar_t	*crosshair;
extern	cvar_t	*crosshairColor;

void DrawString (int x, int y, const char *s);	// draw colorized string; used by menus/console
void SCR_SetLevelshot (const char *name = NULL); // give levelshot name (if name not specified - compute from map name)
void SCR_ShowConsole (bool show, bool noAnim);
void SCR_ToggleConsole (void);
void Key_Message (int key);

//
// scr_cin.c
//
void SCR_PlayCinematic (char *filename);		// begin
bool SCR_DrawCinematic (void);					// play/draw
void SCR_StopCinematic (void);					// stop
