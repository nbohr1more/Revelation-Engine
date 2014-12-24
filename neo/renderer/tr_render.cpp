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

#include "tr_local.h"

/*

  back end scene + lights rendering functions

  */

/*
=================
RB_DrawElementsImmediate

Draws with immediate mode commands, which is going to be very slow.
This should never happen if the vertex cache is operating properly.
=================
*/
void RB_DrawElementsImmediate(const srfTriangles_t *tri) {
	backEnd.pc.c_drawElements++;
	backEnd.pc.c_drawIndexes += tri->numIndexes;
	backEnd.pc.c_drawVertexes += tri->numVerts;
	if (tri->ambientSurface != NULL) {
		if (tri->indexes == tri->ambientSurface->indexes) {
			backEnd.pc.c_drawRefIndexes += tri->numIndexes;
		}
		if (tri->verts == tri->ambientSurface->verts) {
			backEnd.pc.c_drawRefVertexes += tri->numVerts;
		}
	}
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < tri->numIndexes; i++) {
		glTexCoord2fv(tri->verts[tri->indexes[i]].st.ToFloatPtr());
		glVertex3fv(tri->verts[tri->indexes[i]].xyz.ToFloatPtr());
	}
	glEnd();
}

/*
================
RB_DrawElementsWithCounters
================
*/
void RB_DrawElementsWithCounters(const srfTriangles_t *tri) {
	backEnd.pc.c_drawElements++;
	backEnd.pc.c_drawIndexes += tri->numIndexes;
	backEnd.pc.c_drawVertexes += tri->numVerts;
	if (tri->ambientSurface != NULL) {
		if (tri->indexes == tri->ambientSurface->indexes) {
			backEnd.pc.c_drawRefIndexes += tri->numIndexes;
		}
		if (tri->verts == tri->ambientSurface->verts) {
			backEnd.pc.c_drawRefVertexes += tri->numVerts;
		}
	}
	if (tri->indexCache && r_useIndexBuffers.GetBool()) {
		glDrawElements(GL_TRIANGLES,
			r_singleTriangle.GetBool() ? 3 : tri->numIndexes,
			GL_INDEX_TYPE,
			(int *)vertexCache.Position(tri->indexCache));
		backEnd.pc.c_vboIndexes += tri->numIndexes;
	}
	else {
		if (r_useIndexBuffers.GetBool()) {
			vertexCache.UnbindIndex(GL_ELEMENT_ARRAY_BUFFER);
		}
		glDrawElements(GL_TRIANGLES,
			r_singleTriangle.GetBool() ? 3 : tri->numIndexes,
			GL_INDEX_TYPE,
			tri->indexes);
	}
}

/*
================
RB_DrawShadowElementsWithCounters

May not use all the indexes in the surface if caps are skipped
================
*/
void RB_DrawShadowElementsWithCounters(const srfTriangles_t *tri, int numIndexes) {
	backEnd.pc.c_shadowElements++;
	backEnd.pc.c_shadowIndexes += numIndexes;
	backEnd.pc.c_shadowVertexes += tri->numVerts;
	if (tri->indexCache && r_useIndexBuffers.GetBool()) {
		glDrawElements(GL_TRIANGLES,
			r_singleTriangle.GetBool() ? 3 : numIndexes,
			GL_INDEX_TYPE,
			(int *)vertexCache.Position(tri->indexCache));
		backEnd.pc.c_vboIndexes += numIndexes;
	}
	else {
		if (r_useIndexBuffers.GetBool()) {
			vertexCache.UnbindIndex(GL_ELEMENT_ARRAY_BUFFER);
		}
		glDrawElements(GL_TRIANGLES,
			r_singleTriangle.GetBool() ? 3 : numIndexes,
			GL_INDEX_TYPE,
			tri->indexes);
	}
}

/*
===============
RB_RenderTriangleSurface

Sets texcoord and vertex pointers
===============
*/
void RB_RenderTriangleSurface(const srfTriangles_t *tri) {
	if (!tri->ambientCache) {
		RB_DrawElementsImmediate(tri);
		return;
	}
	idDrawVert *ac = (idDrawVert *)vertexCache.Position(tri->ambientCache);
	glVertexPointer(3, GL_FLOAT, sizeof(idDrawVert), ac->xyz.ToFloatPtr());
	glTexCoordPointer(2, GL_FLOAT, sizeof(idDrawVert), ac->st.ToFloatPtr());
	RB_DrawElementsWithCounters(tri);
}

/*
===============
RB_T_RenderTriangleSurface
===============
*/
void RB_T_RenderTriangleSurface(const drawSurf_t *surf) {
	RB_RenderTriangleSurface(surf->geo);
}

/*
===============
RB_EnterWeaponDepthHack
===============
*/
void RB_EnterWeaponDepthHack(void) {
	float			matrix[16];
	const float		depthRange = 0.25f;
	glDepthRange(0.0f, depthRange * 2.0f);
	memcpy(matrix, backEnd.viewDef->projectionMatrix, sizeof(matrix));
	matrix[2] *= idMath::ClampFloat(0.0f, 1.0f, depthRange);
	matrix[6] *= idMath::ClampFloat(0.0f, 1.0f, depthRange);
	matrix[10] *= idMath::ClampFloat(0.0f, 1.0f, depthRange);
	matrix[14] *= idMath::ClampFloat(0.0f, 1.0f, depthRange);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((const GLfloat *)matrix);
	glMatrixMode(GL_MODELVIEW);
}

/*
===============
RB_EnterModelDepthHack
===============
*/
void RB_EnterModelDepthHack(float depth) {
	float	matrix[16];
	glDepthRange(0.0f, 1.0f);
	memcpy(matrix, backEnd.viewDef->projectionMatrix, sizeof(matrix));
	matrix[14] -= idMath::ClampFloat(0.0f, 1.0f, depth);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf((const GLfloat *)matrix);
	glMatrixMode(GL_MODELVIEW);
}

/*
===============
RB_LeaveDepthHack
===============
*/
void RB_LeaveDepthHack(void) {
	glDepthRange(0.0f, 1.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(backEnd.viewDef->projectionMatrix);
	glMatrixMode(GL_MODELVIEW);
}

/*
====================
RB_RenderDrawSurfListWithFunction

The triangle functions can check backEnd.currentSpace != surf->space
to see if they need to perform any new matrix setup.  The modelview
matrix will already have been loaded, and backEnd.currentSpace will
be updated after the triangle function completes.
====================
*/
void RB_RenderDrawSurfListWithFunction(drawSurf_t **drawSurfs, int numDrawSurfs, void(*triFunc_)(const drawSurf_t *)) {
	int					i;
	const drawSurf_t	*drawSurf;
	backEnd.currentSpace = NULL;
	for (i = 0; i < numDrawSurfs; i++) {
		drawSurf = drawSurfs[i];
		// change the matrix if needed
		if (drawSurf->space != backEnd.currentSpace) {
			glLoadMatrixf(drawSurf->space->modelViewMatrix);
		}
		if (drawSurf->space->weaponDepthHack) {
			RB_EnterWeaponDepthHack();
		}
		if (drawSurf->space->modelDepthHack != 0.0f) {
			RB_EnterModelDepthHack(drawSurf->space->modelDepthHack);
		}
		// change the scissor if needed
		if (r_useScissor.GetBool() && !backEnd.currentScissor.Equals(drawSurf->scissorRect)) {
			backEnd.currentScissor = drawSurf->scissorRect;
			GL_Scissor(backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
			backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
			backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
			backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1);
		}
		// render it
		triFunc_(drawSurf);
		if (drawSurf->space->weaponDepthHack || drawSurf->space->modelDepthHack != 0.0f) {
			RB_LeaveDepthHack();
		}
		backEnd.currentSpace = drawSurf->space;
	}
}

/*
======================
RB_RenderDrawSurfChainWithFunction
======================
*/
void RB_RenderDrawSurfChainWithFunction(const drawSurf_t *drawSurfs, void(*triFunc_)(const drawSurf_t *)) {
	const drawSurf_t *drawSurf;
	backEnd.currentSpace = NULL;
	for (drawSurf = drawSurfs; drawSurf; drawSurf = drawSurf->nextOnLight) {
		// change the matrix if needed
		if (drawSurf->space != backEnd.currentSpace) {
			glLoadMatrixf(drawSurf->space->modelViewMatrix);
		}
		if (drawSurf->space->weaponDepthHack) {
			RB_EnterWeaponDepthHack();
		}
		if (drawSurf->space->modelDepthHack) {
			RB_EnterModelDepthHack(drawSurf->space->modelDepthHack);
		}
		// change the scissor if needed
		if (r_useScissor.GetBool() && !backEnd.currentScissor.Equals(drawSurf->scissorRect)) {
			backEnd.currentScissor = drawSurf->scissorRect;
			GL_Scissor(backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
				backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
				backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
				backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1);
		}
		// render it
		triFunc_(drawSurf);
		if (drawSurf->space->weaponDepthHack || drawSurf->space->modelDepthHack != 0.0f) {
			RB_LeaveDepthHack();
		}
		backEnd.currentSpace = drawSurf->space;
	}
}

/*
======================
RB_GetShaderTextureMatrix
======================
*/
void RB_GetShaderTextureMatrix(const float *shaderRegisters, const textureStage_t *texture, float matrix[16]) {
	matrix[0 * 4 + 0] = shaderRegisters[texture->matrix[0][0]];
	matrix[1 * 4 + 0] = shaderRegisters[texture->matrix[0][1]];
	matrix[2 * 4 + 0] = 0.0f;
	matrix[3 * 4 + 0] = shaderRegisters[texture->matrix[0][2]];
	matrix[0 * 4 + 1] = shaderRegisters[texture->matrix[1][0]];
	matrix[1 * 4 + 1] = shaderRegisters[texture->matrix[1][1]];
	matrix[2 * 4 + 1] = 0.0f;
	matrix[3 * 4 + 1] = shaderRegisters[texture->matrix[1][2]];
	// we attempt to keep scrolls from generating incredibly large texture values, but
	// center rotations and center scales can still generate offsets that need to be > 1
	if (matrix[3 * 4 + 0] < -40.0f || matrix[12] > 40.0f) {
		matrix[3 * 4 + 0] -= (int)matrix[3 * 4 + 0];
	}
	if (matrix[13] < -40.0f || matrix[13] > 40.0f) {
		matrix[13] -= (int)matrix[13];
	}
	matrix[0 * 4 + 2] = 0.0f;
	matrix[1 * 4 + 2] = 0.0f;
	matrix[2 * 4 + 2] = 1.0f;
	matrix[3 * 4 + 2] = 0.0f;
	matrix[0 * 4 + 3] = 0.0f;
	matrix[1 * 4 + 3] = 0.0f;
	matrix[2 * 4 + 3] = 0.0f;
	matrix[3 * 4 + 3] = 1.0f;
}

/*
=====================
RB_BakeTextureMatrixIntoTexgen
=====================
*/
void RB_BakeTextureMatrixIntoTexgen(idPlane lightProject[3], const float *textureMatrix) {
	float genMatrix[16];
	float final[16];
	genMatrix[0 * 4 + 0] = lightProject[0][0];
	genMatrix[1 * 4 + 0] = lightProject[0][1];
	genMatrix[2 * 4 + 0] = lightProject[0][2];
	genMatrix[3 * 4 + 0] = lightProject[0][3];
	genMatrix[0 * 4 + 1] = lightProject[1][0];
	genMatrix[1 * 4 + 1] = lightProject[1][1];
	genMatrix[2 * 4 + 1] = lightProject[1][2];
	genMatrix[3 * 4 + 1] = lightProject[1][3];
	genMatrix[0 * 4 + 2] = 0.0f;
	genMatrix[1 * 4 + 2] = 0.0f;
	genMatrix[2 * 4 + 2] = 0.0f;
	genMatrix[3 * 4 + 2] = 0.0f;
	genMatrix[0 * 4 + 3] = lightProject[2][0];
	genMatrix[1 * 4 + 3] = lightProject[2][1];
	genMatrix[2 * 4 + 3] = lightProject[2][2];
	genMatrix[3 * 4 + 3] = lightProject[2][3];
	R_MatrixMultiply(genMatrix, backEnd.lightTextureMatrix, final);
	lightProject[0][0] = final[0 * 4 + 0];
	lightProject[0][1] = final[1 * 4 + 0];
	lightProject[0][2] = final[2 * 4 + 0];
	lightProject[0][3] = final[3 * 4 + 0];
	lightProject[1][0] = final[0 * 4 + 1];
	lightProject[1][1] = final[1 * 4 + 1];
	lightProject[1][2] = final[2 * 4 + 1];
	lightProject[1][3] = final[3 * 4 + 1];
}

/*
======================
RB_LoadShaderTextureMatrix
======================
*/
void RB_LoadShaderTextureMatrix(const float *shaderRegisters, const textureStage_t *texture) {
	float	matrix[16];
	RB_GetShaderTextureMatrix(shaderRegisters, texture, matrix);
	glMatrixMode(GL_TEXTURE);
	glLoadMatrixf(matrix);
	glMatrixMode(GL_MODELVIEW);
}

/*
======================
RB_BindVariableStageImage

Handles generating a cinematic frame if needed
======================
*/
void RB_BindVariableStageImage(const textureStage_t *texture, const float *shaderRegisters) {
	if (texture->cinematic) {
		cinData_t	cin;
		if (r_skipDynamicTextures.GetBool()) {
			globalImages->defaultImage->Bind();
			return;
		}
		// offset time by shaderParm[7] (FIXME: make the time offset a parameter of the shader?)
		// We make no attempt to optimize for multiple identical cinematics being in view, or
		// for cinematics going at a lower framerate than the renderer.
		cin = texture->cinematic->ImageForTime((int)(1000 * (backEnd.viewDef->floatTime + backEnd.viewDef->renderView.shaderParms[11])));
		if (cin.image) {
			globalImages->cinematicImage->UploadScratch(cin.image, cin.imageWidth, cin.imageHeight);
		}
		else {
			globalImages->blackImage->Bind();
		}
	}
	else {
		//FIXME: see why image is invalid
		if (texture->image) {
			texture->image->Bind();
		}
	}
}

//=============================================================================================

/*
=================
RB_DetermineLightScale

Sets:
backEnd.lightScale
backEnd.overBright

Find out how much we are going to need to overscale the lighting, so we
can down modulate the pre-lighting passes.

We only look at light calculations, but an argument could be made that
we should also look at surface evaluations, which would let surfaces
overbright past 1.0
=================
*/
void RB_DetermineLightScale(void) {
	viewLight_t			*vLight;
	const idMaterial	*shader;
	float				max;
	int					i, j, numStages;
	const shaderStage_t	*stage;
	// the light scale will be based on the largest color component of any surface
	// that will be drawn.
	// should we consider separating rgb scales?
	// if there are no lights, this will remain at 1.0, so GUI-only
	// rendering will not lose any bits of precision
	max = 1.0f;
	for (vLight = backEnd.viewDef->viewLights; vLight; vLight = vLight->next) {
		// lights with no surfaces or shaderparms may still be present
		// for debug display
		if (!vLight->localInteractions && !vLight->globalInteractions && !vLight->translucentInteractions) {
			continue;
		}
		shader = vLight->lightShader;
		numStages = shader->GetNumStages();
		for (i = 0; i < numStages; i++) {
			stage = shader->GetStage(i);
			for (j = 0; j < 3; j++) {
				float	v = r_lightScale.GetFloat() * vLight->shaderRegisters[stage->color.registers[j]];
				if (v > max) {
					max = v;
				}
			}
		}
	}
	backEnd.pc.maxLightValue = max;
	backEnd.lightScale = r_lightScale.GetFloat() / max;
	backEnd.overBright = max;
}

/*
=================
RB_BeginDrawingView

Any mirrored or portaled views have already been drawn, so prepare
to actually render the visible surfaces for this view
=================
*/
void RB_BeginDrawingView(void) {
	// set the modelview matrix for the viewer
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(backEnd.viewDef->projectionMatrix);
	glMatrixMode(GL_MODELVIEW);
	// set the window clipping
	GL_Viewport(tr.viewportOffset[0] + backEnd.viewDef->viewport.x1,
		tr.viewportOffset[1] + backEnd.viewDef->viewport.y1,
		backEnd.viewDef->viewport.x2 + 1 - backEnd.viewDef->viewport.x1,
		backEnd.viewDef->viewport.y2 + 1 - backEnd.viewDef->viewport.y1);
	// the scissor may be smaller than the viewport for subviews
	GL_Scissor(tr.viewportOffset[0] + backEnd.viewDef->viewport.x1 + backEnd.viewDef->scissor.x1,
		tr.viewportOffset[1] + backEnd.viewDef->viewport.y1 + backEnd.viewDef->scissor.y1,
		backEnd.viewDef->scissor.x2 + 1 - backEnd.viewDef->scissor.x1,
		backEnd.viewDef->scissor.y2 + 1 - backEnd.viewDef->scissor.y1);
	backEnd.currentScissor = backEnd.viewDef->scissor;
	// ensures that depth writes are enabled for the depth clear
	GL_State(GLS_DEFAULT);
	// we don't have to clear the depth / stencil buffer for 2D rendering
	if (backEnd.viewDef->viewEntitys) {
		glStencilMask(0xff);
		// some cards may have 7 bit stencil buffers, so don't assume this
		// should be 128
		glClearStencil(1 << (glConfig.stencilBits - 1));
		glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_STENCIL_TEST);
	}
	backEnd.glState.faceCulling = -1;		// force face culling to set next time
	GL_Cull(CT_FRONT_SIDED);
}

/*
=================
RB_ClearMatrixInteractions
=================
*/
static void RB_ClearMatrixInteractions(idVec4 matrix[2]) {
	// first stage
	matrix[0][0] = 1.0f;
	matrix[0][1] = 0.0f;
	matrix[0][2] = 0.0f;
	matrix[0][3] = 0.0f;
	// next stage
	matrix[1][0] = 0.0f;
	matrix[1][1] = 1.0f;
	matrix[1][2] = 0.0f;
	matrix[1][3] = 0.0f;
	// thats all she wrote folks
}

/*
==================
R_SetDrawInteractions
==================
*/
void R_SetDrawInteraction(const shaderStage_t *surfaceStage, const float *surfaceRegs, idImage **image, idVec4 matrix[2], float color[4]) {
	*image = surfaceStage->texture.image;
	if (surfaceStage->texture.hasMatrix) {
		matrix[0][0] = surfaceRegs[surfaceStage->texture.matrix[0][0]];
		matrix[0][1] = surfaceRegs[surfaceStage->texture.matrix[0][1]];
		matrix[0][2] = 0.0f;
		matrix[0][3] = surfaceRegs[surfaceStage->texture.matrix[0][2]];
		matrix[1][0] = surfaceRegs[surfaceStage->texture.matrix[1][0]];
		matrix[1][1] = surfaceRegs[surfaceStage->texture.matrix[1][1]];
		matrix[1][2] = 0.0f;
		matrix[1][3] = surfaceRegs[surfaceStage->texture.matrix[1][2]];
		// we attempt to keep scrolls from generating incredibly large texture values, but
		// center rotations and center scales can still generate offsets that need to be > 1
		if (matrix[0][3] < -40.0f || matrix[0][3] > 40.0f) {
			matrix[0][3] -= (int)matrix[0][3];
		}
		if (matrix[1][3] < -40.0f || matrix[1][3] > 40.0f) {
			matrix[1][3] -= (int)matrix[1][3];
		}
	}
	else {
		RB_ClearMatrixInteractions(matrix);
	}
	if( color ) {
		color[0] = idMath::ClampFloat(0.0f, 1.0f, surfaceRegs[surfaceStage->color.registers[0]]);
		color[1] = idMath::ClampFloat(0.0f, 1.0f, surfaceRegs[surfaceStage->color.registers[1]]);
		color[2] = idMath::ClampFloat(0.0f, 1.0f, surfaceRegs[surfaceStage->color.registers[2]]);
		color[3] = idMath::ClampFloat(0.0f, 1.0f, surfaceRegs[surfaceStage->color.registers[3]]);
	}
}

/*
=================
RB_SubmitInteraction
=================
*/
static void RB_SubmitInteraction(drawInteraction_t *din, void(*DrawInteraction)(const drawInteraction_t *)) {
	if (!din->bumpImage) {
		// if no bumpmap then we dont have diffuse or specular either.
		RB_ClearMatrixInteractions(din->bumpMatrix);
		RB_ClearMatrixInteractions(din->specularMatrix);
		RB_ClearMatrixInteractions(din->diffuseMatrix);
		return;
	}
	if (!din->diffuseImage || r_skipDiffuse.GetBool()) {
		RB_ClearMatrixInteractions(din->diffuseMatrix);
		din->diffuseImage = globalImages->blackImage;
	}
	if (!din->specularImage || r_skipSpecular.GetBool() || din->ambientLight) {
		RB_ClearMatrixInteractions(din->specularMatrix);
		din->specularImage = globalImages->blackImage;
	}
	if (!din->bumpImage || r_skipBump.GetBool()) {
		RB_ClearMatrixInteractions(din->bumpMatrix);
		din->bumpImage = globalImages->flatNormalMap;
	}
	DrawInteraction( din );
}

/*
=============
RB_CreateSingleDrawInteractions

This can be used by different draw_* backends to decompose a complex light / surface
interaction into primitive interactions
=============
*/
void RB_CreateSingleDrawInteractions(const drawSurf_t *surf, void(*DrawInteraction)(const drawInteraction_t *)) {
	const idMaterial	*surfaceShader = surf->material;
	const float			*surfaceRegs = surf->shaderRegisters;
	const viewLight_t	*vLight = backEnd.vLight;
	const idMaterial	*lightShader = vLight->lightShader;
	const float			*lightRegs = vLight->shaderRegisters;
	bool				lightDepthBoundsDisabled = false;
	drawInteraction_t	inter;
	if (r_skipInteractions.GetBool() || !surf->geo || !surf->geo->ambientCache) {
		return;
	}
	// change the matrix and light projection vectors if needed
	if (surf->space != backEnd.currentSpace) {
		backEnd.currentSpace = surf->space;
		// Disable the depth bounds test because translucent surfaces don't work with
		// the depth bounds tests since they do not write depth during the depth pass.
		if (r_useDepthBoundsTest.GetBool() && !vLight->translucentInteractions) {
			// turn off the light depth bounds test if this model is rendered with a depth hack
			if (!surf->space->weaponDepthHack && surf->space->modelDepthHack == 0.0f) {
				if (glConfig.depthBoundsTestAvailable && lightDepthBoundsDisabled) {
					GL_DepthBoundsTest(vLight->scissorRect.zmin, vLight->scissorRect.zmax);
				} else if (glConfig.depthClampAvailable && lightDepthBoundsDisabled) {
					glEnable(GL_DEPTH_CLAMP);
				}
				lightDepthBoundsDisabled = false;
			}
			else {
				if (glConfig.depthBoundsTestAvailable && !lightDepthBoundsDisabled) {
					GL_DepthBoundsTest(0.0f, 0.0f);
				} else if (glConfig.depthClampAvailable && !lightDepthBoundsDisabled) {
					glDisable(GL_DEPTH_CLAMP);
				}
				lightDepthBoundsDisabled = true;
			}
		}
		// model-view-projection
		glLoadMatrixf(surf->space->modelViewMatrix);
	}
	// change the scissor if needed
	if (r_useScissor.GetBool() && !backEnd.currentScissor.Equals(surf->scissorRect)) {
		backEnd.currentScissor = surf->scissorRect;
		GL_Scissor(backEnd.viewDef->viewport.x1 + backEnd.currentScissor.x1,
			backEnd.viewDef->viewport.y1 + backEnd.currentScissor.y1,
			backEnd.currentScissor.x2 + 1 - backEnd.currentScissor.x1,
			backEnd.currentScissor.y2 + 1 - backEnd.currentScissor.y1);
	}
	// hack depth range if needed
	if (surf->space->weaponDepthHack) {
		RB_EnterWeaponDepthHack();
	}
	if (surf->space->modelDepthHack) {
		RB_EnterModelDepthHack(surf->space->modelDepthHack);
	}
	inter.surf = surf;
	inter.lightFalloffImage = vLight->falloffImage;
	R_GlobalPointToLocal(surf->space->modelMatrix, vLight->globalLightOrigin, inter.localLightOrigin.ToVec3());
	R_GlobalPointToLocal(surf->space->modelMatrix, backEnd.viewDef->renderView.vieworg, inter.localViewOrigin.ToVec3());
	inter.localLightOrigin[3] = 0.0f;
	inter.localViewOrigin[3] = 1.0f;
	inter.ambientLight = lightShader->IsAmbientLight();
	// the base projections may be modified by texture matrix on light stages
	idPlane lightProject[4];
	for (int i = 0; i < 4; i++) {
		R_GlobalPlaneToLocal(surf->space->modelMatrix, backEnd.vLight->lightProject[i], lightProject[i]);
	}
	for (int lightStageNum = 0; lightStageNum < lightShader->GetNumStages(); lightStageNum++) {
		const shaderStage_t	*lightStage = lightShader->GetStage(lightStageNum);
		// ignore stages that fail the condition
		if (!lightRegs[lightStage->conditionRegister]) {
			continue;
		}
		inter.lightImage = lightStage->texture.image;
		memcpy(inter.lightProjection, lightProject, sizeof(inter.lightProjection));
		// now multiply the texgen by the light texture matrix
		if (lightStage->texture.hasMatrix) {
			RB_GetShaderTextureMatrix(lightRegs, &lightStage->texture, backEnd.lightTextureMatrix);
			RB_BakeTextureMatrixIntoTexgen(reinterpret_cast<class idPlane *>(inter.lightProjection), backEnd.lightTextureMatrix);
		}
		inter.bumpImage = NULL;
		inter.specularImage = NULL;
		inter.diffuseImage = NULL;
		inter.diffuseColor[0] = inter.diffuseColor[1] = inter.diffuseColor[2] = inter.diffuseColor[3] = 0;
		inter.specularColor[0] = inter.specularColor[1] = inter.specularColor[2] = inter.specularColor[3] = 0;
		// backEnd.lightScale is calculated so that lightColor[] will never exceed
		// tr.backEndRendererMaxLight
		float lightColor[4] = {
			backEnd.lightScale * lightRegs[ lightStage->color.registers[0] ],
			backEnd.lightScale * lightRegs[ lightStage->color.registers[1] ],
			backEnd.lightScale * lightRegs[ lightStage->color.registers[2] ],
			lightColor[3] = lightRegs[ lightStage->color.registers[3] ]
		};
		// go through the individual stages
		for (int surfaceStageNum = 0; surfaceStageNum < surfaceShader->GetNumStages(); surfaceStageNum++) {
			const shaderStage_t	*surfaceStage = surfaceShader->GetStage(surfaceStageNum);
			switch (surfaceStage->lighting) {
				case SL_AMBIENT: {
					// ignore ambient stages while drawing interactions
					break;
				}
				case SL_BUMP: {
					// ignore stage that fails the condition
					if (!surfaceRegs[surfaceStage->conditionRegister]) {
						break;
					}
					// draw any previous interaction
					RB_SubmitInteraction(&inter, DrawInteraction);
					inter.diffuseImage = NULL;
					inter.specularImage = NULL;
					R_SetDrawInteraction(surfaceStage, surfaceRegs, &inter.bumpImage, inter.bumpMatrix, NULL);
					break;
				}
				case SL_DIFFUSE: {
					// ignore stage that fails the condition
					if (!surfaceRegs[surfaceStage->conditionRegister]) {
						break;
					}
					if (inter.diffuseImage) {
						RB_SubmitInteraction(&inter, DrawInteraction);
					}
					R_SetDrawInteraction(surfaceStage, surfaceRegs, &inter.diffuseImage, inter.diffuseMatrix, inter.diffuseColor.ToFloatPtr());
					inter.diffuseColor[0] *= lightColor[0];
					inter.diffuseColor[1] *= lightColor[1];
					inter.diffuseColor[2] *= lightColor[2];
					inter.diffuseColor[3] *= lightColor[3];
					inter.vertexColor = surfaceStage->vertexColor;
					break;
				}
				case SL_SPECULAR: {
					// ignore stage that fails the condition
					if (!surfaceRegs[surfaceStage->conditionRegister]) {
						break;
					}
					if (inter.specularImage) {
						RB_SubmitInteraction(&inter, DrawInteraction);
					}
					R_SetDrawInteraction(surfaceStage, surfaceRegs, &inter.specularImage, inter.specularMatrix, inter.specularColor.ToFloatPtr());
					inter.specularColor[0] *= lightColor[0];
					inter.specularColor[1] *= lightColor[1];
					inter.specularColor[2] *= lightColor[2];
					inter.specularColor[3] *= lightColor[3];
					inter.vertexColor = surfaceStage->vertexColor;
					break;
				}
			}
		}
		// draw the final interaction
		RB_SubmitInteraction(&inter, DrawInteraction);
	}
	// unhack depth range if needed
	if (surf->space->weaponDepthHack || surf->space->modelDepthHack != 0.0f) {
		RB_LeaveDepthHack();
	}
	// Clamp Z to a 0..1 value
	if (r_useDepthBoundsTest.GetBool() && glConfig.depthBoundsTestAvailable) {
		if (lightDepthBoundsDisabled) {
			GL_DepthBoundsTest(vLight->scissorRect.zmin, vLight->scissorRect.zmax);
		}
		else {
			GL_DepthBoundsTest(0.0f, 0.0f);
		}
	} else if (glConfig.depthClampAvailable) {
		if (lightDepthBoundsDisabled) {
			glEnable(GL_DEPTH_CLAMP);
		}
		else {
			glDisable(GL_DEPTH_CLAMP);
		}
	}
}

/*
=============
RB_DrawView
=============
*/
void RB_DrawView(const void *data) {
	const drawSurfsCommand_t	*cmd;
	cmd = (const drawSurfsCommand_t *)data;
	backEnd.viewDef = cmd->viewDef;
	// we will need to do a new copyTexSubImage of the screen
	// when a SS_POST_PROCESS material is used
	backEnd.currentRenderCopied = false;
	// Yank in depth buffer to
	backEnd.currentDepthCopied = false;
	// if there aren't any drawsurfs, do nothing
	if (!backEnd.viewDef->numDrawSurfs) {
		return;
	}
	// skip render bypasses everything that has models, assuming
	// them to be 3D views, but leaves 2D rendering visible
	if (r_skipRender.GetBool() && backEnd.viewDef->viewEntitys) {
		return;
	}
	// skip render context sets the wgl context to NULL,
	// which should factor out the API cost, under the assumption
	// that all gl calls just return if the context isn't valid
	if (r_skipRenderContext.GetBool() && backEnd.viewDef->viewEntitys) {
		GLimp_DeactivateContext();
	}
	backEnd.pc.c_surfaces += backEnd.viewDef->numDrawSurfs;
	RB_ShowOverdraw();
	// render the scene, jumping to the hardware specific interaction renderers
	RB_STD_DrawView();
	// restore the context for 2D drawing if we were stubbing it out
	if (r_skipRenderContext.GetBool() && backEnd.viewDef->viewEntitys) {
		GLimp_ActivateContext();
		RB_SetDefaultGLState();
	}
}