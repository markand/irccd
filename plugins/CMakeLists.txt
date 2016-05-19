#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

project(plugins)

add_custom_target(
	all-plugins ALL
	COMMENT "Irccd plugins"
)

set_target_properties(
	all-plugins
	PROPERTIES
		FOLDER meta
)

foreach (plugin ${IRCCD_PLUGINS})
	string(TOUPPER ${plugin} optname)

	if (WITH_PLUGIN_${optname})
		# 1. Configure the plugin and install it.
		irccd_define_plugin(${plugin}/${plugin}.js)

		# 2. Build documentation.
		if (WITH_HTML)
			set(basedocdir ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR})

			file(RELATIVE_PATH baseurl ${basedocdir}/plugin ${basedocdir})
	
			pandoc(
				OUTPUT ${basedocdir}/plugin/${plugin}.html
				SOURCES ${plugin}/${plugin}.md
				TEMPLATE ${resources_SOURCE_DIR}/template.html
				DEPENDS
					${resources_SOURCE_DIR}/template.html
					docs-resources
				ARGS -Vguide
				VARIABLE baseurl:${baseurl}
				FROM markdown TO html5
				STANTALONE MAKE_DIRECTORY TOC
			)
	
			list(APPEND outputs ${basedocdir}/plugin/${plugin}.html)
			install(FILES ${basedocdir}/plugin/${plugin}.html DESTINATION ${WITH_DOCDIR}/plugin)
		endif ()

		add_custom_target(
			plugin-${plugin}
			SOURCES
				${outputs}
				${plugin}/${plugin}.js
				${plugin}/${plugin}.md
		)

		set_target_properties(
			plugin-${plugin}
			PROPERTIES
				PROJECT_LABEL ${plugin}
				FOLDER plugins
		)

		add_dependencies(all-plugins plugin-${plugin})
	endif ()
endforeach ()
