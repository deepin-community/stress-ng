/*
 * Copyright (C) 2024      Colin Ian King
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include <sys/acl.h>
#include <acl/libacl.h>

/* The following functions from libacl are used by stress-ng */

static void *aio_funcs[] = {
	acl_add_perm,
	acl_calc_mask,
	acl_clear_perms,
	acl_create_entry,
	acl_delete_entry,
	acl_free,
	acl_get_entry,
	acl_get_file,
	acl_get_perm,
	acl_get_permset,
	acl_get_tag_type,
	acl_init,
	acl_set_file,
	acl_set_permset,
	acl_set_qualifier,
	acl_valid,
};

int main(void)
{
	return 0;
}
