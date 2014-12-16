/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "../idlib/precompiled.h"
#pragma hdrstop

void	RadiantInit(void) {
	common->Printf("The level editor Radiant only runs on Win32\n");
}
void	RadiantShutdown(void) {}
void	RadiantRun(void) {}
void	RadiantPrint(const char *text) {}
void	RadiantSync(const char *mapName, const idVec3 &viewOrg, const idAngles &viewAngles) {}

void	LightEditorInit(const idDict *spawnArgs) {
	common->Printf("The internal Light Editor was removed\n");
}
void	LightEditorShutdown(void) {}
void	LightEditorRun(void) {}

void	SoundEditorInit(const idDict *spawnArgs) {
	common->Printf("The internal Sound Editor was removed\n");
}
void	SoundEditorShutdown(void) {}
void	SoundEditorRun(void) {}

void	AFEditorInit(const idDict *spawnArgs) {
	common->Printf("The internal Articulated Figure Editor was removed\n");
}
void	AFEditorShutdown(void) {}
void	AFEditorRun(void) {}

void	ParticleEditorInit(const idDict *spawnArgs) {
	common->Printf("The internal Particle Editor was removed\n");
}
void	ParticleEditorShutdown(void) {}
void	ParticleEditorRun(void) {}

void	ScriptEditorInit(const idDict *spawnArgs) {
	common->Printf("The internal Script Editor was removed\n");
}
void	ScriptEditorShutdown(void) {}
void	ScriptEditorRun(void) {}

void	DeclBrowserInit(const idDict *spawnArgs) {
	common->Printf("The internal Declaration Browser was removed\n");
}
void	DeclBrowserShutdown(void) {}
void	DeclBrowserRun(void) {}
void	DeclBrowserReloadDeclarations(void) {}

void	GUIEditorInit(void) {
	common->Printf("The internal GUI Editor was removed\n");
}
void	GUIEditorShutdown(void) {}
void	GUIEditorRun(void) {}
bool	GUIEditorHandleMessage(void *msg) {
	return false;
}

void	PDAEditorInit(const idDict *spawnArgs) {
	common->Printf("The internal PDA editor was removed\n");
}

void	MaterialEditorInit() {
	common->Printf("The internal Material editor was removed\n");
}
void	MaterialEditorPrintConsole(const char *text) {}