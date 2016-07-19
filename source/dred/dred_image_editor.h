// Copyright (C) 2016 David Reid. See included LICENSE file.

#ifndef DRED_NO_IMAGE_EDITOR

#define DRED_CONTROL_TYPE_IMAGE_EDITOR  "dred.editor.image"

typedef dred_editor dred_image_editor;

dred_image_editor* dred_image_editor_create(dred_context* pDred, dred_control* pParent, const char* filePathAbsolute);
void dred_image_editor_delete(dred_image_editor* pImageEditor);

// Sets the scale of the image. This is how zooming works.
void dred_image_editor_set_image_scale(dred_image_editor* pImageEditor, float scale);

// Retrieves the scale of the image.
float dred_image_editor_get_image_scale(dred_image_editor* pImageEditor);

#endif