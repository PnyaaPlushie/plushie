
find_package( PlushieStartup QUIET REQUIRED )
find_package( unity          QUIET REQUIRED )

add_executable(PlushieStartup_Test_ExitCode0
	expect_exit_code.c)

set_target_properties(PlushieStartup_Test_ExitCode0 PROPERTIES
	FOLDER                  "plushie/plushie_startup/tests"
	PREFIX                  "lib"
	IMPORT_PREFIX           "lib"
	DEBUG_POSTFIX           "Dbg"
	RELEASE_POSTFIX         "Rel"
	RELWITHDEBINFO_POSTFIX  "RelDbg"
	MINSIZEREL_POSTFIX      "RelMin"
	)

plushie_startup_apply_target_options(PlushieStartup_Test_ExitCode0)