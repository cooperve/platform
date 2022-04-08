/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Tim Copperfield.
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Tim Copperfield <timecop@network.email.ne.jp>
 *   Christian Dywan <christian@twotoasts.de>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef CursorGtk_h
#define CursorGtk_h

#include "Cursor.h"

/*
 These cursors are copied from Mozilla code:
  http://lxr.mozilla.org/mozilla1.8/source/widget/src/gtk2/nsGtkCursors.h
*/

/* MOZ_CURSOR_VERTICAL_TEXT */
static const char moz_vertical_text_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00,
  0x06, 0x60, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0x06, 0x60, 0x00, 0x00,
  0x02, 0x40, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const char moz_vertical_text_mask_bits[] = {
  0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00,
  0xff, 0xff, 0x00, 0x00, 0xfe, 0x7f, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00,
  0x0f, 0xf0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* MOZ_CURSOR_CONTEXT_MENU */
static const char moz_menu_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x0c, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
  0x7c, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xfc, 0xfd, 0x00, 0x00,
  0xfc, 0xff, 0x00, 0x00, 0x7c, 0x84, 0x00, 0x00, 0x6c, 0xfc, 0x00, 0x00,
  0xc4, 0x84, 0x00, 0x00, 0xc0, 0xfc, 0x00, 0x00, 0x80, 0x85, 0x00, 0x00,
  0x80, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const char moz_menu_mask_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00,
  0x1e, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00,
  0xfe, 0x00, 0x00, 0x00, 0xfe, 0xfd, 0x00, 0x00, 0xfe, 0xff, 0x01, 0x00,
  0xfe, 0xff, 0x01, 0x00, 0xfe, 0xff, 0x01, 0x00, 0xfe, 0xfe, 0x01, 0x00,
  0xee, 0xff, 0x01, 0x00, 0xe4, 0xff, 0x01, 0x00, 0xc0, 0xff, 0x01, 0x00,
  0xc0, 0xff, 0x01, 0x00, 0x80, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* MOZ_CURSOR_COPY */
static const char moz_copy_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x0c, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
  0x7c, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xfc, 0x01, 0x00, 0x00,
  0xfc, 0x03, 0x00, 0x00, 0x7c, 0x30, 0x00, 0x00, 0x6c, 0x30, 0x00, 0x00,
  0xc4, 0xfc, 0x00, 0x00, 0xc0, 0xfc, 0x00, 0x00, 0x80, 0x31, 0x00, 0x00,
  0x80, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const char moz_copy_mask_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00,
  0x1e, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00,
  0xfe, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x00, 0x00, 0xfe, 0x03, 0x00, 0x00,
  0xfe, 0x37, 0x00, 0x00, 0xfe, 0x7b, 0x00, 0x00, 0xfe, 0xfc, 0x00, 0x00,
  0xee, 0xff, 0x01, 0x00, 0xe4, 0xff, 0x01, 0x00, 0xc0, 0xff, 0x00, 0x00,
  0xc0, 0x7b, 0x00, 0x00, 0x80, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* MOZ_CURSOR_ALIAS */
static const char moz_alias_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x0c, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
  0x7c, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xfc, 0x01, 0x00, 0x00,
  0xfc, 0x03, 0x00, 0x00, 0x7c, 0xf0, 0x00, 0x00, 0x6c, 0xe0, 0x00, 0x00,
  0xc4, 0xf0, 0x00, 0x00, 0xc0, 0xb0, 0x00, 0x00, 0x80, 0x19, 0x00, 0x00,
  0x80, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const char moz_alias_mask_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00,
  0x1e, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00,
  0xfe, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x00, 0x00, 0xfe, 0x03, 0x00, 0x00,
  0xfe, 0xf7, 0x00, 0x00, 0xfe, 0xfb, 0x01, 0x00, 0xfe, 0xf0, 0x01, 0x00,
  0xee, 0xf9, 0x01, 0x00, 0xe4, 0xf9, 0x01, 0x00, 0xc0, 0xbf, 0x00, 0x00,
  0xc0, 0x3f, 0x00, 0x00, 0x80, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* MOZ_CURSOR_ZOOM_IN */
static const char moz_zoom_in_bits[] = {
  0xf0, 0x00, 0x00, 0x00, 0x0c, 0x03, 0x00, 0x00, 0x02, 0x04, 0x00, 0x00,
  0x62, 0x04, 0x00, 0x00, 0x61, 0x08, 0x00, 0x00, 0xf9, 0x09, 0x00, 0x00,
  0xf9, 0x09, 0x00, 0x00, 0x61, 0x08, 0x00, 0x00, 0x62, 0x04, 0x00, 0x00,
  0x02, 0x04, 0x00, 0x00, 0x0c, 0x0f, 0x00, 0x00, 0xf0, 0x1c, 0x00, 0x00,
  0x00, 0x38, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00,
  0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const char moz_zoom_in_mask_bits[] = {
  0xf0, 0x00, 0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0xfe, 0x07, 0x00, 0x00,
  0xfe, 0x07, 0x00, 0x00, 0xff, 0x0f, 0x00, 0x00, 0xff, 0x0f, 0x00, 0x00,
  0xff, 0x0f, 0x00, 0x00, 0xff, 0x0f, 0x00, 0x00, 0xfe, 0x07, 0x00, 0x00,
  0xfe, 0x07, 0x00, 0x00, 0xfc, 0x0f, 0x00, 0x00, 0xf0, 0x1c, 0x00, 0x00,
  0x00, 0x38, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00,
  0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* MOZ_CURSOR_ZOOM_OUT */
static const char moz_zoom_out_bits[] = {
  0xf0, 0x00, 0x00, 0x00, 0x0c, 0x03, 0x00, 0x00, 0x02, 0x04, 0x00, 0x00,
  0x02, 0x04, 0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0xf9, 0x09, 0x00, 0x00,
  0xf9, 0x09, 0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0x02, 0x04, 0x00, 0x00,
  0x02, 0x04, 0x00, 0x00, 0x0c, 0x0f, 0x00, 0x00, 0xf0, 0x1c, 0x00, 0x00,
  0x00, 0x38, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00,
  0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const char moz_zoom_out_mask_bits[] = {
  0xf0, 0x00, 0x00, 0x00, 0xfc, 0x03, 0x00, 0x00, 0xfe, 0x07, 0x00, 0x00,
  0xfe, 0x07, 0x00, 0x00, 0xff, 0x0f, 0x00, 0x00, 0xff, 0x0f, 0x00, 0x00,
  0xff, 0x0f, 0x00, 0x00, 0xff, 0x0f, 0x00, 0x00, 0xfe, 0x07, 0x00, 0x00,
  0xfe, 0x07, 0x00, 0x00, 0xfc, 0x0f, 0x00, 0x00, 0xf0, 0x1c, 0x00, 0x00,
  0x00, 0x38, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00,
  0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* MOZ_CURSOR_NOT_ALLOWED */
static const char moz_not_allowed_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x80, 0x1f, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0x00,
  0xf0, 0xf0, 0x00, 0x00, 0x38, 0xc0, 0x01, 0x00, 0x7c, 0x80, 0x03, 0x00,
  0xec, 0x00, 0x03, 0x00, 0xce, 0x01, 0x07, 0x00, 0x86, 0x03, 0x06, 0x00,
  0x06, 0x07, 0x06, 0x00, 0x06, 0x0e, 0x06, 0x00, 0x06, 0x1c, 0x06, 0x00,
  0x0e, 0x38, 0x07, 0x00, 0x0c, 0x70, 0x03, 0x00, 0x1c, 0xe0, 0x03, 0x00,
  0x38, 0xc0, 0x01, 0x00, 0xf0, 0xf0, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0x00,
  0x80, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const char moz_not_allowed_mask_bits[] = {
  0x80, 0x1f, 0x00, 0x00, 0xe0, 0x7f, 0x00, 0x00, 0xf0, 0xff, 0x00, 0x00,
  0xf8, 0xff, 0x01, 0x00, 0xfc, 0xf0, 0x03, 0x00, 0xfe, 0xc0, 0x07, 0x00,
  0xfe, 0x81, 0x07, 0x00, 0xff, 0x83, 0x0f, 0x00, 0xcf, 0x07, 0x0f, 0x00,
  0x8f, 0x0f, 0x0f, 0x00, 0x0f, 0x1f, 0x0f, 0x00, 0x0f, 0x3e, 0x0f, 0x00,
  0x1f, 0xfc, 0x0f, 0x00, 0x1e, 0xf8, 0x07, 0x00, 0x3e, 0xf0, 0x07, 0x00,
  0xfc, 0xf0, 0x03, 0x00, 0xf8, 0xff, 0x01, 0x00, 0xf0, 0xff, 0x00, 0x00,
  0xe0, 0x7f, 0x00, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* MOZ_CURSOR_SPINNING */
static const char moz_spinning_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x0c, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00,
  0x7c, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0xfc, 0x01, 0x00, 0x00,
  0xfc, 0x3b, 0x00, 0x00, 0x7c, 0x38, 0x00, 0x00, 0x6c, 0x54, 0x00, 0x00,
  0xc4, 0xdc, 0x00, 0x00, 0xc0, 0x44, 0x00, 0x00, 0x80, 0x39, 0x00, 0x00,
  0x80, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const char moz_spinning_mask_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00,
  0x1e, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00,
  0xfe, 0x00, 0x00, 0x00, 0xfe, 0x01, 0x00, 0x00, 0xfe, 0x3b, 0x00, 0x00,
  0xfe, 0x7f, 0x00, 0x00, 0xfe, 0x7f, 0x00, 0x00, 0xfe, 0xfe, 0x00, 0x00,
  0xee, 0xff, 0x01, 0x00, 0xe4, 0xff, 0x00, 0x00, 0xc0, 0x7f, 0x00, 0x00,
  0xc0, 0x7f, 0x00, 0x00, 0x80, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* MOZ_CURSOR_NONE */
static const char moz_none_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const char moz_none_mask_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* MOZ_CURSOR_HAND_GRAB */
static const char moz_hand_grab_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00,
  0x60, 0x39, 0x00, 0x00, 0x90, 0x49, 0x00, 0x00, 0x90, 0x49, 0x01, 0x00,
  0x20, 0xc9, 0x02, 0x00, 0x20, 0x49, 0x02, 0x00, 0x58, 0x40, 0x02, 0x00,
  0x64, 0x00, 0x02, 0x00, 0x44, 0x00, 0x01, 0x00, 0x08, 0x00, 0x01, 0x00,
  0x10, 0x00, 0x01, 0x00, 0x10, 0x80, 0x00, 0x00, 0x20, 0x80, 0x00, 0x00,
  0x40, 0x40, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const char moz_hand_grab_mask_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x60, 0x3f, 0x00, 0x00,
  0xf0, 0x7f, 0x00, 0x00, 0xf8, 0xff, 0x01, 0x00, 0xf8, 0xff, 0x03, 0x00,
  0xf0, 0xff, 0x07, 0x00, 0xf8, 0xff, 0x07, 0x00, 0xfc, 0xff, 0x07, 0x00,
  0xfe, 0xff, 0x07, 0x00, 0xfe, 0xff, 0x03, 0x00, 0xfc, 0xff, 0x03, 0x00,
  0xf8, 0xff, 0x03, 0x00, 0xf8, 0xff, 0x01, 0x00, 0xf0, 0xff, 0x01, 0x00,
  0xe0, 0xff, 0x00, 0x00, 0xc0, 0xff, 0x00, 0x00, 0xc0, 0xff, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/* MOZ_CURSOR_HAND_GRABBING */
static const char moz_hand_grabbing_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xc0, 0x36, 0x00, 0x00, 0x20, 0xc9, 0x00, 0x00, 0x20, 0x40, 0x01, 0x00,
  0x40, 0x00, 0x01, 0x00, 0x60, 0x00, 0x01, 0x00, 0x10, 0x00, 0x01, 0x00,
  0x10, 0x00, 0x01, 0x00, 0x10, 0x80, 0x00, 0x00, 0x20, 0x80, 0x00, 0x00,
  0x40, 0x40, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x80, 0x40, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static const  char moz_hand_grabbing_mask_bits[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x36, 0x00, 0x00,
  0xe0, 0xff, 0x00, 0x00, 0xf0, 0xff, 0x01, 0x00, 0xf0, 0xff, 0x03, 0x00,
  0xe0, 0xff, 0x03, 0x00, 0xf0, 0xff, 0x03, 0x00, 0xf8, 0xff, 0x03, 0x00,
  0xf8, 0xff, 0x03, 0x00, 0xf8, 0xff, 0x01, 0x00, 0xf0, 0xff, 0x01, 0x00,
  0xe0, 0xff, 0x00, 0x00, 0xc0, 0xff, 0x00, 0x00, 0xc0, 0x7f, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

enum CustomCursorType {
    CustomCursorCopy = 0,
    CustomCursorAlias,
    CustomCursorContextMenu,
    CustomCursorZoomIn,
    CustomCursorZoomOut,
    CustomCursorVerticalText,
    CustomCursorNoDrop,
    CustomCursorProgress,
    CustomCursorNone,
    CustomCursorGrab,
    CustomCursorGrabbing,
};

typedef struct {
    const char* name;
    const char* bits;
    const char* mask_bits;
    int hot_x;
    int hot_y;
} CustomCursor;

// create custom pixmap cursor from cursors in nsGTKCursorData.h
static const CustomCursor CustomCursors[] = {
    { "copy", moz_copy_bits, moz_copy_mask_bits, 2, 2 },
    { "alias", moz_alias_bits, moz_alias_mask_bits, 2, 2 },
    { "context-menu", moz_menu_bits, moz_menu_mask_bits, 2, 2 },
    { "zoom-in", moz_zoom_in_bits, moz_zoom_in_mask_bits, 6, 6 },
    { "zoom-out", moz_zoom_out_bits, moz_zoom_out_mask_bits, 6, 6 },
    { "vertical-text", moz_vertical_text_bits, moz_vertical_text_mask_bits, 8, 4 },
    { "dnd-no-drop", moz_not_allowed_bits, moz_not_allowed_mask_bits, 9,  9 },
    { "progress", moz_spinning_bits, moz_spinning_mask_bits, 2,  2},
    { "none", moz_none_bits, moz_none_mask_bits, 0, 0 },
    { "grab", moz_hand_grab_bits, moz_hand_grab_mask_bits, 10, 10 },
    { "grabbing", moz_hand_grabbing_bits, moz_hand_grabbing_mask_bits, 10, 10 }
};

#endif // CursorGtk_h
