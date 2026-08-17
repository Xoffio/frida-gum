#include "gumjs/gumscriptbackend.h"

#include <stdio.h>
#include <string.h>

static void on_message (const gchar * message, GBytes * data,
    gpointer user_data);

int
main (int argc, char * argv[])
{
  GumScriptBackend * backend;
  GError * error = NULL;
  GumScript * script;
  GMainContext * context;

  gum_init_embedded ();

  backend = gum_script_backend_obtain_qjs ();

  script = gum_script_backend_create_sync (backend, "test",
      "var buf = Memory.alloc(16);\n"
      "buf.writeByteArray([\n"
      "  0xAA, 0x13, 0x37,\n"
      "  0xBB,\n"
      "  0xCC, 0x13, 0x37,\n"
      "  0xDD, 0x13, 0x37,\n"
      "  0x00\n"
      "]);\n"
      "\n"
      "var results;\n"
      "\n"
      "results = Memory.scanSync(buf, 11, \"?? 13 37\");\n"
      "send('leading ?? matches: ' + results.length);\n"
      "results.forEach(function (m) {\n"
      "  send('  offset: ' + m.address.sub(buf));\n"
      "});\n"
      "\n"
      "results = Memory.scanSync(buf, 11, \"13 37 ??\");\n"
      "send('trailing ?? matches: ' + results.length);\n"
      "results.forEach(function (m) {\n"
      "  send('  offset: ' + m.address.sub(buf));\n"
      "});\n"
      "\n"
      "results = Memory.scanSync(buf, 11, \"?? 13 37 ??\");\n"
      "send('leading+trailing ?? matches: ' + results.length);\n"
      "results.forEach(function (m) {\n"
      "  send('  offset: ' + m.address.sub(buf));\n"
      "});\n",
      NULL, NULL, &error);

  if (error != NULL)
  {
    fprintf (stderr, "Script creation failed: %s\n", error->message);
    g_error_free (error);
    gum_deinit_embedded ();
    return 1;
  }

  gum_script_set_message_handler (script, on_message, NULL, NULL);

  gum_script_load_sync (script, NULL);

  context = g_main_context_get_thread_default ();
  while (g_main_context_pending (context))
    g_main_context_iteration (context, FALSE);

  gum_script_unload_sync (script, NULL);
  g_object_unref (script);

  gum_deinit_embedded ();

  return 0;
}

static void
on_message (const gchar * message,
            GBytes * data,
            gpointer user_data)
{
  JsonParser * parser;
  JsonObject * root;
  const gchar * type;

  parser = json_parser_new ();
  json_parser_load_from_data (parser, message, -1, NULL);
  root = json_node_get_object (json_parser_get_root (parser));
  type = json_object_get_string_member (root, "type");

  if (strcmp (type, "send") == 0)
  {
    const gchar * payload =
        json_object_get_string_member (root, "payload");
    printf ("%s\n", payload);
  }
  else
  {
    printf ("message: %s\n", message);
  }

  g_object_unref (parser);
}
