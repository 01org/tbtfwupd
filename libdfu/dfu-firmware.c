/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2015 Richard Hughes <richard@hughsie.com>
 *
 * Licensed under the GNU Lesser General Public License Version 2.1
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

/**
 * SECTION:dfu-firmware
 * @short_description: Object representing a DFU firmware
 *
 * This object allows reading and writing DFU-suffix files.
 */

#include "config.h"

#include <fwupd.h>
#include <string.h>
#include <stdio.h>

#include "dfu-common.h"
#include "dfu-firmware.h"

static void dfu_firmware_finalize			 (GObject *object);

/**
 * DfuFirmwarePrivate:
 *
 * Private #DfuFirmware data
 **/
typedef struct {
	GPtrArray		*images;
	guint16			 vid;
	guint16			 pid;
	guint16			 release;
	DfuFirmwareFormat	 format;
} DfuFirmwarePrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DfuFirmware, dfu_firmware, G_TYPE_OBJECT)
#define GET_PRIVATE(o) (dfu_firmware_get_instance_private (o))

/**
 * dfu_firmware_class_init:
 **/
static void
dfu_firmware_class_init (DfuFirmwareClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = dfu_firmware_finalize;
}

/**
 * dfu_firmware_init:
 **/
static void
dfu_firmware_init (DfuFirmware *firmware)
{
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	priv->vid = 0xffff;
	priv->pid = 0xffff;
	priv->release = 0xffff;
	priv->images = g_ptr_array_new_with_free_func ((GDestroyNotify) g_object_unref);
}

/**
 * dfu_firmware_finalize:
 **/
static void
dfu_firmware_finalize (GObject *object)
{
	DfuFirmware *firmware = DFU_FIRMWARE (object);
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);

	g_ptr_array_unref (priv->images);

	G_OBJECT_CLASS (dfu_firmware_parent_class)->finalize (object);
}

/**
 * dfu_firmware_new:
 *
 * Creates a new DFU firmware object.
 *
 * Return value: a new #DfuFirmware
 *
 * Since: 0.5.4
 **/
DfuFirmware *
dfu_firmware_new (void)
{
	DfuFirmware *firmware;
	firmware = g_object_new (DFU_TYPE_FIRMWARE, NULL);
	return firmware;
}

/**
 * dfu_firmware_get_image:
 * @firmware: a #DfuFirmware
 * @alt_setting: an alternative setting, typically 0x00
 *
 * Gets an image from the firmware file.
 *
 * Return value: (transfer none): a #DfuImage, or %NULL for not found
 *
 * Since: 0.5.4
 **/
DfuImage *
dfu_firmware_get_image (DfuFirmware *firmware, guint8 alt_setting)
{
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	DfuImage *im;
	guint i;

	g_return_val_if_fail (DFU_IS_FIRMWARE (firmware), NULL);

	/* find correct image */
	for (i = 0; i < priv->images->len; i++) {
		im = g_ptr_array_index (priv->images, i);
		if (dfu_image_get_alt_setting (im) == alt_setting)
			return im;
	}
	return NULL;
}

/**
 * dfu_firmware_get_images:
 * @firmware: a #DfuFirmware
 *
 * Gets all the images contained in this firmware file.
 *
 * Return value: (transfer none) (element-type DfuImage): list of images
 *
 * Since: 0.5.4
 **/
GPtrArray *
dfu_firmware_get_images (DfuFirmware *firmware)
{
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	g_return_val_if_fail (DFU_IS_FIRMWARE (firmware), NULL);
	return priv->images;
}

/**
 * dfu_firmware_add_image:
 * @firmware: a #DfuFirmware
 * @image: a #DfuImage
 *
 * Adds an image to the list of images.
 *
 * Since: 0.5.4
 **/
void
dfu_firmware_add_image (DfuFirmware *firmware, DfuImage *image)
{
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	g_return_if_fail (DFU_IS_FIRMWARE (firmware));
	g_return_if_fail (DFU_IS_IMAGE (image));
	g_ptr_array_add (priv->images, g_object_ref (image));
}

/**
 * dfu_firmware_get_vid:
 * @firmware: a #DfuFirmware
 *
 * Gets the vendor ID.
 *
 * Return value: a vendor ID, or 0xffff for unset
 *
 * Since: 0.5.4
 **/
guint16
dfu_firmware_get_vid (DfuFirmware *firmware)
{
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	g_return_val_if_fail (DFU_IS_FIRMWARE (firmware), 0xffff);
	return priv->vid;
}

/**
 * dfu_firmware_get_pid:
 * @firmware: a #DfuFirmware
 *
 * Gets the product ID.
 *
 * Return value: a product ID, or 0xffff for unset
 *
 * Since: 0.5.4
 **/
guint16
dfu_firmware_get_pid (DfuFirmware *firmware)
{
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	g_return_val_if_fail (DFU_IS_FIRMWARE (firmware), 0xffff);
	return priv->pid;
}

/**
 * dfu_firmware_get_release:
 * @firmware: a #DfuFirmware
 *
 * Gets the device ID.
 *
 * Return value: a device ID, or 0xffff for unset
 *
 * Since: 0.5.4
 **/
guint16
dfu_firmware_get_release (DfuFirmware *firmware)
{
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	g_return_val_if_fail (DFU_IS_FIRMWARE (firmware), 0xffff);
	return priv->release;
}

/**
 * dfu_firmware_get_format:
 * @firmware: a #DfuFirmware
 *
 * Gets the DFU version.
 *
 * Return value: a version, or 0x0 for unset
 *
 * Since: 0.5.4
 **/
guint16
dfu_firmware_get_format (DfuFirmware *firmware)
{
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	g_return_val_if_fail (DFU_IS_FIRMWARE (firmware), 0xffff);
	return priv->format;
}

/**
 * dfu_firmware_set_vid:
 * @firmware: a #DfuFirmware
 * @vid: vendor ID, or 0xffff for unset
 *
 * Sets the vendor ID.
 *
 * Since: 0.5.4
 **/
void
dfu_firmware_set_vid (DfuFirmware *firmware, guint16 vid)
{
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	g_return_if_fail (DFU_IS_FIRMWARE (firmware));
	priv->vid = vid;
}

/**
 * dfu_firmware_set_pid:
 * @firmware: a #DfuFirmware
 * @pid: product ID, or 0xffff for unset
 *
 * Sets the product ID.
 *
 * Since: 0.5.4
 **/
void
dfu_firmware_set_pid (DfuFirmware *firmware, guint16 pid)
{
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	g_return_if_fail (DFU_IS_FIRMWARE (firmware));
	priv->pid = pid;
}

/**
 * dfu_firmware_set_release:
 * @firmware: a #DfuFirmware
 * @release: device ID, or 0xffff for unset
 *
 * Sets the device ID.
 *
 * Since: 0.5.4
 **/
void
dfu_firmware_set_release (DfuFirmware *firmware, guint16 release)
{
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	g_return_if_fail (DFU_IS_FIRMWARE (firmware));
	priv->release = release;
}

/**
 * dfu_firmware_set_format:
 * @firmware: a #DfuFirmware
 * @format: a #DfuFirmwareFormat, e.g. %DFU_FIRMWARE_FORMAT_DFUSE
 *
 * Sets the DFU version in BCD format.
 *
 * Since: 0.5.4
 **/
void
dfu_firmware_set_format (DfuFirmware *firmware, DfuFirmwareFormat format)
{
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	g_return_if_fail (DFU_IS_FIRMWARE (firmware));
	priv->format = format;
}

typedef struct __attribute__((packed)) {
	guint16		release;
	guint16		pid;
	guint16		vid;
	guint16		ver;
	guint8		sig[3];
	guint8		len;
	guint32		crc;
} DfuFirmwareFooter;

static guint32 _crctbl[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d };

/**
 * dfu_firmware_generate_crc32:
 **/
static guint32
dfu_firmware_generate_crc32 (const guint8 *data, gsize length)
{
	guint i;
	guint32 accum = 0xffffffff;
	for (i = 0; i < length; i++)
		accum = _crctbl[(accum^data[i]) & 0xff] ^ (accum >> 8);
	return accum;
}


/**
 * dfu_firmware_inhx32_parse_uint8:
 **/
static guint8
dfu_firmware_inhx32_parse_uint8 (const gchar *data, guint pos)
{
	gchar buffer[3];
	buffer[0] = data[pos+0];
	buffer[1] = data[pos+1];
	buffer[2] = '\0';
	return g_ascii_strtoull (buffer, NULL, 16);
}

#define	DFU_INHX32_RECORD_TYPE_DATA		0
#define	DFU_INHX32_RECORD_TYPE_EOF		1
#define	DFU_INHX32_RECORD_TYPE_EXTENDED		4

/**
 * dfu_firmware_parse_inhx32:
 **/
static GBytes *
dfu_firmware_parse_inhx32 (GBytes *in, GError **error)
{
	gchar *ptr;
	gint checksum;
	gint end;
	gint i;
	gint offset = 0;
	guint8 data_tmp;
	guint addr32 = 0;
	guint addr32_last = 0;
	guint addr_high = 0;
	guint addr_low = 0;
	guint j;
	guint len_tmp;
	guint type;
	g_autoptr(GString) string = NULL;
	const gchar *in_buffer = g_bytes_get_data (in, NULL);

	g_return_val_if_fail (in != NULL, NULL);

	/* only if set */
	string = g_string_new ("");
	while (TRUE) {

		/* length, 16-bit address, type */
		if (sscanf (&in_buffer[offset], ":%02x%04x%02x",
			    &len_tmp, &addr_low, &type) != 3) {
			g_set_error_literal (error,
					     FWUPD_ERROR,
					     FWUPD_ERROR_INTERNAL,
					     "invalid inhx32 syntax");
			return NULL;
		}

		/* position of checksum */
		end = offset + 9 + len_tmp * 2;

		/* verify checksum */
		checksum = 0;
		for (i = offset + 1; i < end; i += 2) {
			data_tmp = dfu_firmware_inhx32_parse_uint8 (in_buffer, i);
			checksum = (checksum + (0x100 - data_tmp)) & 0xff;
		}
		if (dfu_firmware_inhx32_parse_uint8 (in_buffer, end) != checksum)  {
			g_set_error_literal (error,
					     FWUPD_ERROR,
					     FWUPD_ERROR_INTERNAL,
					     "invalid checksum");
			return NULL;
		}

		/* process different record types */
		switch (type) {
		case DFU_INHX32_RECORD_TYPE_DATA:
			/* if not contiguous with previous record */
			if ((addr_high + addr_low) != addr32)
				addr32 = addr_high + addr_low;

			/* parse bytes from line */
			for (i = offset + 9; i < end; i += 2) {
				/* any holes in the hex record */
				len_tmp = addr32 - addr32_last;
				if (addr32_last > 0x0 && len_tmp > 1) {
					for (j = 1; j < len_tmp; j++) {
						g_debug ("filling address 0x%04x",
							 addr32_last + j);
						/* although 0xff might be clearer,
						 * we can't write 0xffff to pic14 */
						g_string_append_c (string, 0x00);
					}
				}
				/* write into buf */
				data_tmp = dfu_firmware_inhx32_parse_uint8 (in_buffer, i);
				g_string_append_c (string, data_tmp);
				g_debug ("writing address 0x%04x", addr32);
				addr32_last = addr32++;
			}
			break;
		case DFU_INHX32_RECORD_TYPE_EOF:
			break;
		case DFU_INHX32_RECORD_TYPE_EXTENDED:
			if (sscanf (&in_buffer[offset+9], "%04x", &addr_high) != 1) {
				g_set_error_literal (error,
						     FWUPD_ERROR,
						     FWUPD_ERROR_INTERNAL,
						     "invalid hex syntax");
				return NULL;
			}
			addr_high <<= 16;
			addr32 = addr_high + addr_low;
			break;
		default:
			g_set_error_literal (error,
					     FWUPD_ERROR,
					     FWUPD_ERROR_INTERNAL,
					     "invalid record type");
			return NULL;
		}

		/* advance to start of next line */
		ptr = strchr (&in_buffer[end+2], ':');
		if (ptr == NULL)
			break;
		offset = ptr - in_buffer;
	}

	/* save data */
	return g_bytes_new (string->str, string->len);
}

/* DfuSe header */
typedef struct __attribute__((packed)) {
	guint8		 sig[5];
	guint8		 ver;
	guint32		 image_size;
	guint8		 targets;
} DfuSePrefix;

/* DfuSe image header */
typedef struct __attribute__((packed)) {
	guint8		 sig[6];
	guint8		 alternate_setting;
	guint32		 target_named;
	gchar		 target_name[255];
	guint32		 target_size;
	guint32		 elements;
} DfuSeImagePrefix;

/* DfuSe element header */
typedef struct __attribute__((packed)) {
	guint32		 address;
	guint32		 size;
} DfuSeElementPrefix;

/**
 * dfu_firmware_add_inhx32:
 **/
static gboolean
dfu_firmware_add_inhx32 (DfuFirmware *firmware, GBytes *bytes, GError **error)
{
	g_autoptr(DfuImage) image = NULL;
	g_autoptr(GBytes) contents = NULL;

	/* parse hex file */
	contents = dfu_firmware_parse_inhx32 (bytes, error);
	if (contents == NULL)
		return FALSE;

	/* add single image */
	image = dfu_image_new ();
	dfu_image_set_contents (image, contents);
	dfu_firmware_add_image (firmware, image);
	return TRUE;
}

/**
 * dfu_firmware_add_binary:
 **/
static gboolean
dfu_firmware_add_binary (DfuFirmware *firmware, GBytes *bytes, GError **error)
{
	g_autoptr(DfuImage) image = NULL;
	image = dfu_image_new ();
	dfu_image_set_contents (image, bytes);
	dfu_firmware_add_image (firmware, image);
	return TRUE;
}

/**
 * dfu_firmware_add_dfuse:
 **/
static gboolean
dfu_firmware_add_dfuse (DfuFirmware *firmware, GBytes *bytes, GError **error)
{
	DfuSePrefix *prefix;
	gsize len;
	guint32 offset = sizeof(DfuSePrefix);
	guint8 *data;
	guint i;

	/* check the prefix (BE) */
	data = (guint8 *) g_bytes_get_data (bytes, &len);
	prefix = (DfuSePrefix *) data;
	if (memcmp (prefix->sig, "DfuSe", 5) != 0) {
		g_set_error_literal (error,
				     FWUPD_ERROR,
				     FWUPD_ERROR_INTERNAL,
				     "invalid DfuSe prefix");
		return FALSE;
	}

	/* check the version */
	if (prefix->ver != 0x01) {
		g_set_error (error,
			     FWUPD_ERROR,
			     FWUPD_ERROR_INTERNAL,
			     "invalid DfuSe version, got %02x",
			     prefix->ver);
		return FALSE;
	}

	/* check image size */
	if (GUINT32_FROM_LE (prefix->image_size) != len) {
		g_set_error (error,
			     FWUPD_ERROR,
			     FWUPD_ERROR_INTERNAL,
			     "invalid DfuSe image size, got %u, expected %lu",
			     GUINT32_FROM_LE (prefix->image_size),
			     len);
		return FALSE;
	}

	/* parse the image targets */
	for (i = 0; i < prefix->targets; i++) {
		DfuSeImagePrefix *im;
		guint j;
		guint32 image_offset = offset + sizeof(DfuSeImagePrefix);
		g_autoptr(DfuImage) image = NULL;
		g_autoptr(GBytes) contents = NULL;

		/* verify image target */
		im = (DfuSeImagePrefix *) (data + offset);
		if (memcmp (im->sig, "Target", 6) != 0) {
			g_set_error_literal (error,
					     FWUPD_ERROR,
					     FWUPD_ERROR_INTERNAL,
					     "invalid DfuSe target signature");
			return FALSE;
		}

		/* create new image */
		image = dfu_image_new ();
		dfu_image_set_alt_setting (image, im->alternate_setting);
		if (im->target_named == 0x01)
			dfu_image_set_name (image, im->target_name);
		contents = g_bytes_new_from_bytes (bytes,
						   offset,
						   GUINT32_FROM_LE (im->target_size));
		dfu_image_set_contents (image, contents);
		dfu_firmware_add_image (firmware, image);

		/* TODO: parse elements */
		for (j = 0; j < im->elements; j++) {
			DfuSeElementPrefix *element = NULL;
			element = (DfuSeElementPrefix *) (data + image_offset);
			g_print("element address 0x%04x\n", GUINT32_FROM_LE (element->address));
			g_print("element size 0x%04x\n", GUINT32_FROM_LE (element->size));
			image_offset += element->size + sizeof(DfuSeElementPrefix);
		}
		offset = im->target_size;
	}
	return TRUE;
}

/**
 * dfu_firmware_parse_data:
 * @firmware: a #DfuFirmware
 * @bytes: raw firmware data
 * @flags: optional flags, e.g. %DFU_FIRMWARE_PARSE_FLAG_NO_CRC_TEST
 * @error: a #GError, or %NULL
 *
 * Parses firmware data which may have an optional DFU suffix.
 *
 * Return value: %TRUE for success
 *
 * Since: 0.5.4
 **/
gboolean
dfu_firmware_parse_data (DfuFirmware *firmware, GBytes *bytes,
			 DfuFirmwareParseFlags flags, GError **error)
{
	DfuFirmwareFooter *ftr;
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	gsize len;
	guint32 crc_new;
	guint8 *data;
	g_autoptr(GBytes) contents = NULL;

	g_return_val_if_fail (DFU_IS_FIRMWARE (firmware), FALSE);
	g_return_val_if_fail (bytes != NULL, FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	/* sanity check */
	g_assert_cmpint(sizeof(DfuSePrefix), ==, 11);
	g_assert_cmpint(sizeof(DfuSeImagePrefix), ==, 274);
	g_assert_cmpint(sizeof(DfuSeElementPrefix), ==, 8);

	/* set defaults */
	priv->vid = 0xffff;
	priv->pid = 0xffff;
	priv->release = 0xffff;

	/* this is inhx32 */
	data = (guint8 *) g_bytes_get_data (bytes, &len);
	if (data[0] == ':')
		return dfu_firmware_add_inhx32 (firmware, bytes, error);

	/* too small to be a DFU file */
	if (len < 16)
		return dfu_firmware_add_binary (firmware, bytes, error);

	/* check for DFU signature */
	ftr = (DfuFirmwareFooter *) &data[len-sizeof(DfuFirmwareFooter)];
	if (memcmp (ftr->sig, "UFD", 3) != 0)
		return dfu_firmware_add_binary (firmware, bytes, error);

	/* check version */
	priv->format = GUINT16_FROM_LE (ftr->ver);
	if ((flags & DFU_FIRMWARE_PARSE_FLAG_NO_VERSION_TEST) == 0) {
		if (priv->format != DFU_FIRMWARE_FORMAT_DFU_1_0 &&
		    priv->format != DFU_FIRMWARE_FORMAT_DFUSE) {
			g_set_error (error,
				     FWUPD_ERROR,
				     FWUPD_ERROR_INTERNAL,
				     "version check failed, got %04x",
				     priv->format);
			return FALSE;
		}
	}

	/* verify the checksum */
	if ((flags & DFU_FIRMWARE_PARSE_FLAG_NO_CRC_TEST) == 0) {
		crc_new = dfu_firmware_generate_crc32 (data, len - 4);
		if (GUINT32_FROM_LE (ftr->crc) != crc_new) {
			g_set_error (error,
				     FWUPD_ERROR,
				     FWUPD_ERROR_INTERNAL,
				     "CRC failed, expected %04x, got %04x",
				     crc_new, GUINT32_FROM_LE (ftr->crc));
			return FALSE;
		}
	}

	/* set from footer */
	dfu_firmware_set_vid (firmware, GUINT16_FROM_LE (ftr->vid));
	dfu_firmware_set_pid (firmware, GUINT16_FROM_LE (ftr->pid));
	dfu_firmware_set_release (firmware, GUINT16_FROM_LE (ftr->release));

	/* parse DfuSe prefix */
	contents = g_bytes_new_from_bytes (bytes, 0, len - GUINT16_FROM_LE (ftr->len));
	if (priv->format == DFU_FIRMWARE_FORMAT_DFUSE)
		return dfu_firmware_add_dfuse (firmware, contents, error);

	/* just copy old-plain DFU file */
	return dfu_firmware_add_binary (firmware, contents, error);
}

/**
 * dfu_firmware_parse_file:
 * @firmware: a #DfuFirmware
 * @file: a #GFile to load and parse
 * @flags: optional flags, e.g. %DFU_FIRMWARE_PARSE_FLAG_NO_CRC_TEST
 * @cancellable: a #GCancellable, or %NULL
 * @error: a #GError, or %NULL
 *
 * Parses a DFU firmware, which may contain an optional footer.
 *
 * Return value: %TRUE for success
 *
 * Since: 0.5.4
 **/
gboolean
dfu_firmware_parse_file (DfuFirmware *firmware, GFile *file,
			 DfuFirmwareParseFlags flags,
			 GCancellable *cancellable, GError **error)
{
	gchar *contents = NULL;
	gsize length = 0;
	g_autoptr(GBytes) bytes = NULL;

	g_return_val_if_fail (DFU_IS_FIRMWARE (firmware), FALSE);
	g_return_val_if_fail (G_IS_FILE (file), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	if (!g_file_load_contents (file, cancellable, &contents,
				   &length, NULL, error))
		return FALSE;
	bytes = g_bytes_new_take (contents, length);
	return dfu_firmware_parse_data (firmware, bytes, flags, error);
}

/**
 * dfu_firmware_write_data:
 * @firmware: a #DfuFirmware
 * @error: a #GError, or %NULL
 *
 * Writes DFU data to a data blob with a DFU-specific footer.
 *
 * Return value: (transfer none): firmware data
 *
 * Since: 0.5.4
 **/
GBytes *
dfu_firmware_write_data (DfuFirmware *firmware, GError **error)
{
	DfuFirmwareFooter *ftr;
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	DfuImage *image;
	GBytes *contents;
	const guint8 *data;
	gsize length = 0;
	guint8 *buf;

	g_return_val_if_fail (DFU_IS_FIRMWARE (firmware), NULL);
	g_return_val_if_fail (error == NULL || *error == NULL, NULL);

	/* at least one image */
	if (priv->images == 0) {
		g_set_error_literal (error,
				     FWUPD_ERROR,
				     FWUPD_ERROR_INTERNAL,
				     "no image data to write");
		return FALSE;
	}

	/* DFU only supports one image */
	if (priv->images->len > 1 &&
	    priv->format != DFU_FIRMWARE_FORMAT_DFUSE) {
		g_set_error_literal (error,
				     FWUPD_ERROR,
				     FWUPD_ERROR_INTERNAL,
				     "only DfuSe format supports multiple images");
		return FALSE;
	}

	/* copy firmware contents */
	image = dfu_firmware_get_image (firmware, 0); //FIXME: this will not work for DfuSe images
	g_assert (image != NULL);
	contents = dfu_image_get_contents (image);
	g_assert (contents != NULL);
	data = g_bytes_get_data (contents, &length);
	buf = g_malloc0 (length + 0x10);
	memcpy (buf, data, length);

	/* set up LE footer */
	ftr = (DfuFirmwareFooter *) &buf[length];
	ftr->release = GUINT16_TO_LE (priv->release);
	ftr->pid = GUINT16_TO_LE (priv->pid);
	ftr->vid = GUINT16_TO_LE (priv->vid);
	ftr->ver = GUINT16_TO_LE (0x0100);
	ftr->len = GUINT16_TO_LE (0x10);
	memcpy(ftr->sig, "UFD", 3);
	ftr->crc = dfu_firmware_generate_crc32 (buf, length + 12);

	/* return all data */
	return g_bytes_new (buf, length + 0x10);
}

/**
 * dfu_firmware_write_file:
 * @firmware: a #DfuFirmware
 * @file: a #GFile
 * @cancellable: a #GCancellable, or %NULL
 * @error: a #GError, or %NULL
 *
 * Writes a DFU firmware with the optional footer.
 *
 * Return value: %TRUE for success
 *
 * Since: 0.5.4
 **/
gboolean
dfu_firmware_write_file (DfuFirmware *firmware, GFile *file,
			 GCancellable *cancellable, GError **error)
{
	const guint8 *data;
	gsize length = 0;
	g_autoptr(GBytes) bytes = NULL;

	g_return_val_if_fail (DFU_IS_FIRMWARE (firmware), FALSE);
	g_return_val_if_fail (G_IS_FILE (file), FALSE);
	g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

	/* get blob */
	bytes = dfu_firmware_write_data (firmware, error);
	if (bytes == NULL)
		return FALSE;

	/* save to firmware */
	data = g_bytes_get_data (bytes, &length);
	return g_file_replace_contents (file,
					(const gchar *) data,
					length,
					NULL,
					FALSE,
					G_FILE_CREATE_NONE,
					NULL,
					cancellable,
					error);
}

/**
 * dfu_firmware_to_string:
 * @firmware: a #DfuFirmware
 *
 * Returns a string representaiton of the object.
 *
 * Return value: NULL terminated string, or %NULL for invalid
 *
 * Since: 0.5.4
 **/
gchar *
dfu_firmware_to_string (DfuFirmware *firmware)
{
	DfuFirmwarePrivate *priv = GET_PRIVATE (firmware);
	DfuImage *image;
	GString *str;
	guint i;

	g_return_val_if_fail (DFU_IS_FIRMWARE (firmware), NULL);

	str = g_string_new ("");
	g_string_append_printf (str, "vid:      0x%04x\n", priv->vid);
	g_string_append_printf (str, "pid:      0x%04x\n", priv->pid);
	g_string_append_printf (str, "release:  0x%04x\n", priv->release);
	g_string_append_printf (str, "format:   %s [0x%04x]\n",
				dfu_firmware_format_to_string (priv->format),
				priv->format);
	for (i = 0; i < priv->images->len; i++) {
		g_autofree gchar *tmp = NULL;
		image = g_ptr_array_index (priv->images, i);
		tmp = dfu_image_to_string (image);
		g_string_append_printf (str, "=== IMAGE %i ===\n", i);
		g_string_append_printf (str, "%s\n", tmp);
	}

	g_string_truncate (str, str->len - 1);
	return g_string_free (str, FALSE);
}

/**
 * dfu_firmware_format_to_string:
 * @format: a #DfuFirmwareFormat, e.g. %DFU_FIRMWARE_FORMAT_DFU_1_0
 *
 * Returns a string representaiton of the format.
 *
 * Return value: NULL terminated string, or %NULL for invalid
 *
 * Since: 0.5.4
 **/
const gchar *
dfu_firmware_format_to_string (DfuFirmwareFormat format)
{
	if (format == DFU_FIRMWARE_FORMAT_DFU_1_0)
		return "DFU";
	if (format == DFU_FIRMWARE_FORMAT_DFUSE)
		return "DfuSe";
	return NULL;
}
