/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Croma X error handling */

/*
 * Copyright (C) 2001 Havoc Pennington, error trapping inspired by CDK
 * code copyrighted by the CTK team.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <config.h>
#include "errors.h"
#include "display-private.h"
#include <errno.h>
#include <stdlib.h>
#include <cdk/cdk.h>
#include <cdk/cdkx.h>
#include <ctk/ctk.h>

void
meta_error_trap_push (MetaDisplay *display)
{
  cdk_x11_display_error_trap_push (cdk_x11_lookup_xdisplay (meta_display_get_xdisplay (display)));
}

void
meta_error_trap_pop (MetaDisplay *display,
                     gboolean     last_request_was_roundtrip)
{
  cdk_x11_display_error_trap_pop_ignored (cdk_x11_lookup_xdisplay (meta_display_get_xdisplay (display)));
}

int
meta_error_trap_pop_with_return  (MetaDisplay *display,
                                  gboolean     last_request_was_roundtrip)
{
  return cdk_x11_display_error_trap_pop (cdk_x11_lookup_xdisplay (meta_display_get_xdisplay (display)));
}

