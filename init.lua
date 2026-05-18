project = {
	name = "nytrogen_test",
	-- sources = { "systemv_comp_test.ny" },
	-- extra_libs = { "gtk-3", "gobject-2.0", "glib-2.0" },
	sources = { "stress_test.ny" },
	settings = {
		verbose = true,
		debug = true,
		clean = true,
		assembler = "nasm",
	},
}
