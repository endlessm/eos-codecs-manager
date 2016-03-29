/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 * Copyright (C) 2008 Bastien Nocera <bnocera@redhat.com>
 * Copyright (C) 2008 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2016 Endless Mobile, Inc.
 *
 * Helper tool for the installation of missing GStreamer codecs for EOS,
 * heavily based on the implementation from the PackageKit project. See
 * https://github.com/hughsie/PackageKit/blob/master/contrib/gstreamer-plugin
 *
 * Licensed under the GNU General Public License Version 2
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <gst/gst.h>
#include <gst/pbutils/install-plugins.h>
#include <string.h>
#include <sys/utsname.h>

#include <eosmetrics/eosmetrics.h>

/* Recorded by calling gst_install_plugins_async() whenever a GStreamer-based
 * application has tried to install a missing codec via this script. The information
 * reported will be a tuple of 4 strings and one dictionary containing the relevant
 * data about the origin of the event and the specific details about the missing codec.
 *
 * See pkg_gst_report_to_eos_metrics() for more details.
 */
#define EOS_CODECS_MANAGER_MISSING_CODEC "74ceec37-1f66-486e-99b0-d39b23daa113"

typedef struct _EosGstCodecInfo {
	gchar        *gstreamer_version;
	gchar        *app_name;
	gchar        *type_name;
	GstStructure *structure;
} EosGstCodecInfo;

enum {
	FIELD_VERSION = 0,
	FIELD_LAYER,
	FIELD_VARIANT,
	FIELD_SYSTEMSTREAM
};

static EosGstCodecInfo *
eos_gst_parse_codec (const gchar *codec)
{
	GstStructure *s;
	EosGstCodecInfo *info = NULL;
	g_autofree gchar *caps = NULL;
	g_autofree gchar *type_name = NULL;
	g_auto(GStrv) split = NULL;
	g_auto(GStrv) ss = NULL;

	split = g_strsplit (codec, "|", -1);
	if (split == NULL || g_strv_length (split) != 5) {
		g_warning ("eos-gst-codec-install: not a GStreamer codec line");
		return NULL;
	}
	if (g_strcmp0 (split[0], "gstreamer") != 0) {
		g_warning ("eos-gst-codec-install: not a GStreamer codec request");
		return NULL;
	}
	if (g_strcmp0 (split[1], "0.10") != 0 &&
	    g_strcmp0 (split[1], "1.0") != 0) {
		g_warning ("eos-gst-codec-install: not recognised GStreamer version");
		return NULL;
	}

	if (g_str_has_prefix (split[4], "uri") != FALSE) {
		/* split uri */
		ss = g_strsplit (split[4], " ", 2);
		info = g_new0 (EosGstCodecInfo, 1);
		info->app_name = g_strdup (split[2]);
		info->type_name = g_strdup (ss[0]);
		return info;
	}

	/* split */
	ss = g_strsplit (split[4], "-", 2);
	type_name = g_strdup (ss[0]);
	caps = g_strdup (ss[1]);

	s = gst_structure_from_string (caps, NULL);
	if (s == NULL) {
		g_warning ("eos-gst-codec-install: failed to parse caps: %s", caps);
		return NULL;
	}

	/* remove fields that are almost always just MIN-MAX of some sort
	 * in order to make the caps look less messy */
	gst_structure_remove_field (s, "pixel-aspect-ratio");
	gst_structure_remove_field (s, "framerate");
	gst_structure_remove_field (s, "channels");
	gst_structure_remove_field (s, "width");
	gst_structure_remove_field (s, "height");
	gst_structure_remove_field (s, "rate");
	gst_structure_remove_field (s, "depth");
	gst_structure_remove_field (s, "clock-rate");
	gst_structure_remove_field (s, "bitrate");

	info = g_new0 (EosGstCodecInfo, 1);
	info->gstreamer_version = g_strdup (split[1]);
	info->app_name = g_strdup (split[2]);
	info->type_name = g_strdup (type_name);
	info->structure = s;
	return info;
}

static int
eos_gst_field_get_type (const gchar *field_name)
{
	if (g_strrstr (field_name, "version") != NULL)
		return FIELD_VERSION;
	if (g_strcmp0 (field_name, "layer") == 0)
		return FIELD_LAYER;
	if (g_strcmp0 (field_name, "systemstream") == 0)
		return FIELD_SYSTEMSTREAM;
	if (g_strcmp0 (field_name, "variant") == 0)
		return FIELD_VARIANT;
	return -1;
}

static gint
eos_gst_fields_type_compare (const gchar *a, const gchar *b)
{
	gint a_type, b_type;

	a_type = eos_gst_field_get_type (a);
	b_type = eos_gst_field_get_type (b);
	if (a_type < b_type)
		return -1;
	if (b_type < a_type)
		return 1;
	return 0;
}

static GVariantBuilder *
eos_gst_structure_to_dictionary (GstStructure *s)
{
	GVariantBuilder *builder;
	GVariant *dict;
	guint i, num_fields;
	GList *l;

	g_autoptr(GList) fields = NULL;
	num_fields = gst_structure_n_fields (s);
	fields = NULL;

	for (i = 0; i < num_fields; i++) {
		const gchar *field_name;

		field_name = gst_structure_nth_field_name (s, i);
		if (eos_gst_field_get_type (field_name) < 0) {
			g_message ("eos-gst-codec-install: ignoring field named %s", field_name);
			continue;
		}

		fields = g_list_insert_sorted (fields, g_strdup (field_name), (GCompareFunc) eos_gst_fields_type_compare);
	}

	builder = g_variant_builder_new (G_VARIANT_TYPE_VARDICT);
	for (l = fields; l != NULL; l = l->next) {
		gchar *field_name;
		GType type;

		field_name = l->data;
		type = gst_structure_get_field_type (s, field_name);
		g_message ("eos-gst-codec-install: field is: %s, type: %s", field_name, g_type_name (type));

		if (type == G_TYPE_INT) {
			gint value;
			gst_structure_get_int (s, field_name, &value);
			g_variant_builder_add (builder, "{sv}", field_name, g_variant_new_int32 (value));
		} else if (type == G_TYPE_BOOLEAN) {
			gboolean value;
			gst_structure_get_boolean (s, field_name, &value);
			g_variant_builder_add (builder, "{sv}", field_name, g_variant_new_boolean (value));
		} else if (type == G_TYPE_STRING) {
			const gchar *value;
			value = gst_structure_get_string (s, field_name);
			g_variant_builder_add (builder, "{sv}", field_name, g_variant_new_string (value));
		} else {
			g_warning ("eos-gst-codec-install: unhandled type! %s", g_type_name (type));
		}

		g_free (field_name);
	}

	return builder;
}

static void
eos_gst_codec_free (EosGstCodecInfo *codec)
{
	g_free (codec->gstreamer_version);
	g_free (codec->app_name);
	g_free (codec->type_name);
	if (codec->structure)
		gst_structure_free (codec->structure);
	g_free (codec);
}

/**
 * eos_gst_report_missing_codec:
 * @codec_info: a #EosGstCodecInfo
 *
 * Report information about one missing codec to EOS's metrics system.
 *
 * It will report the relevant information as a '(ssssa{sv})' tuple, containing:
 *  - version: version of the GStreamer API (e.g. "1.0")
 *  - source: name of the originating application (e.g. "totem")
 *  - type: type of the missing GStreamer element (e.g. "decoder")
 *  - name: name of the codec (e.g. "video/x-h264", "audio/mpeg")
 *  - extra: additional details (e.g. "mpegversion=4", for "audio/mpeg")
 */
static void
eos_gst_report_missing_codec (EosGstCodecInfo *codec_info, EmtrEventRecorder *recorder)
{
	if (codec_info->structure != NULL) {
		const gchar *codec_name = gst_structure_get_name (codec_info->structure);
		GVariantBuilder *codec_extra = eos_gst_structure_to_dictionary (codec_info->structure);
		GVariant *payload = g_variant_new ("(ssssa{sv})",
						   codec_info->gstreamer_version,
						   codec_info->app_name,
						   codec_info->type_name,
						   codec_name,
						   codec_extra);
		g_variant_builder_unref (codec_extra);

		/* Report payload to the metrics server. */
		g_autofree gchar *payload_str = g_variant_print (payload, FALSE);
		emtr_event_recorder_record_event_sync (recorder, EOS_CODECS_MANAGER_MISSING_CODEC, payload);
		g_message ("eos-gst-codec-install: Reporting to EOS metrics system: %s", payload_str);
	} else {
		g_warning ("eos-gst-codec-install: Not enough information to report missing codec!");
	}
}

/**
 * eos_gst_report_missing_codecs:
 * @codecs_info: a #GPtrArray containing a #EosGstCodecInfo for each missing codec
 *
 * Report information about missing codecs by calling eos_gst_report_missing_codec().
 */
static void
eos_gst_report_missing_codecs (GPtrArray *codecs_info)
{
	/* Report event to EOS's metrics system, one missing codec at a time. */
	EmtrEventRecorder *recorder = emtr_event_recorder_get_default ();
	g_ptr_array_foreach (codecs_info, (GFunc) eos_gst_report_missing_codec, recorder);
}

int
main (int argc, gchar **argv)
{
	GOptionContext *context;
	guint i;
	guint len;
	gboolean ret;
	gchar **codecs = NULL;
	gint xid = 0;
	gchar *resource;
	g_autoptr(GError) error = NULL;
	g_autofree gchar *desktop_id = NULL;
	g_autofree gchar *interaction = NULL;
	g_autofree gchar *startup_id = NULL;
	g_autoptr(GPtrArray) array = NULL;
	g_auto(GStrv) resources = NULL;

	const GOptionEntry options[] = {
		{ "transient-for", '\0', 0, G_OPTION_ARG_INT, &xid, "The XID of the parent window", NULL },
		{ "desktop-id", '\0', 0, G_OPTION_ARG_STRING, &desktop_id, "The desktop ID of the calling application", NULL },
		{ "interaction", '\0', 0, G_OPTION_ARG_STRING, &interaction, "Interaction mode specifying which UI elements should be shown", NULL },
		{ "startup-notification-id", '\0', 0, G_OPTION_ARG_STRING, &startup_id, "The startup notification ID for focus stealing prevention", NULL },
		{ G_OPTION_REMAINING, '\0', 0, G_OPTION_ARG_FILENAME_ARRAY, &codecs, "GStreamer install infos", NULL },
		{ NULL }
	};

	gst_init (&argc, &argv);

	context = g_option_context_new ("Install missing codecs");
	g_option_context_add_main_entries (context, options, NULL);

	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_print ("%s\nRun '%s --help' to see a full list of available command line options.\n",
			 error->message, argv[0]);
		return GST_INSTALL_PLUGINS_ERROR;
	}
	if (codecs == NULL) {
		g_print ("Missing codecs information\n");
		g_print ("Run 'with --help' to see a full list of available command line options.\n");
		return GST_INSTALL_PLUGINS_ERROR;
	}

	/* this is our parent window */
	g_message ("eos-gst-codec-install: xid = %i", xid);
	g_message ("eos-gst-codec-install: desktop_id = %s", desktop_id);

	array = g_ptr_array_new_with_free_func ((GDestroyNotify) eos_gst_codec_free);
	len = g_strv_length (codecs);

	/* Parse parameters into a GPtrArray of EosGstCodecInfo. */
	for (i = 0; i < len; i++) {
		EosGstCodecInfo *info = eos_gst_parse_codec (codecs[i]);
		if (info == NULL) {
			g_message ("Skipping %s", codecs[i]);
			continue;
		}

		g_ptr_array_add (array, info);
	}

	/* nothing parsed */
	if (array->len == 0) {
		g_warning ("No codec lines could be parsed");
		return GST_INSTALL_PLUGINS_ERROR;
	}

	/* Report all missing codecs to the EOS metrics system. */
	eos_gst_report_missing_codecs (array);
	g_ptr_array_free (array, TRUE);

	/* We are not installing codecs at all in EOS for now, just
	 * reporting to the metrics server, so we always fail here.
	 */
	return GST_INSTALL_PLUGINS_NOT_FOUND;
}
