project = {
	name = "gtk_test",
	-- sources = { "gtk.ny" },
	sources = { "stress_test.ny", "tests/test3.ny" },
	-- sources = { "tests/test_namespaces.ny" },
	extra_libs = { "gtk-3", "gobject-2.0", "glib-2.0" },
	settings = {
		verbose = true,
		debug = true,
		clean = true,
		assembler = "nasm",
	},
}
