#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2019 David Demelier <markand@malikania.fr>
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

project(html)

set(IRCCD_WITH_HTML_TEMPLATE ${html_SOURCE_DIR}/template.html
	CACHE STRING "HTML marker template")

set(
	HTML_JS_SOURCES
	js/event/onCommand.md
	js/event/onConnect.md
	js/event/onDisconnect.md
	js/event/onInvite.md
	js/event/onJoin.md
	js/event/onKick.md
	js/event/onLoad.md
	js/event/onMe.md
	js/event/onMessage.md
	js/event/onMode.md
	js/event/onNames.md
	js/event/onNick.md
	js/event/onNotice.md
	js/event/onPart.md
	js/event/onReload.md
	js/event/onTopic.md
	js/event/onUnload.md
	js/event/onWhois.md
	js/index.md
	js/module/Irccd.Directory/index.md
	js/module/Irccd.Directory/Irccd.Directory.find.md
	js/module/Irccd.Directory/Irccd.Directory.mkdir.md
	js/module/Irccd.Directory/Irccd.Directory.prototype.constructor.md
	js/module/Irccd.Directory/Irccd.Directory.prototype.find.md
	js/module/Irccd.Directory/Irccd.Directory.prototype.remove.md
	js/module/Irccd.Directory/Irccd.Directory.remove.md
	js/module/Irccd.ElapsedTimer/index.md
	js/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.constructor.md
	js/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.elapsed.md
	js/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.pause.md
	js/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.reset.md
	js/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.restart.md
	js/module/Irccd.File/index.md
	js/module/Irccd.File/Irccd.File.basename.md
	js/module/Irccd.File/Irccd.File.dirname.md
	js/module/Irccd.File/Irccd.File.exists.md
	js/module/Irccd.File/Irccd.File.prototype.basename.md
	js/module/Irccd.File/Irccd.File.prototype.close.md
	js/module/Irccd.File/Irccd.File.prototype.constructor.md
	js/module/Irccd.File/Irccd.File.prototype.dirname.md
	js/module/Irccd.File/Irccd.File.prototype.lines.md
	js/module/Irccd.File/Irccd.File.prototype.readline.md
	js/module/Irccd.File/Irccd.File.prototype.read.md
	js/module/Irccd.File/Irccd.File.prototype.remove.md
	js/module/Irccd.File/Irccd.File.prototype.seek.md
	js/module/Irccd.File/Irccd.File.prototype.stat.md
	js/module/Irccd.File/Irccd.File.prototype.tell.md
	js/module/Irccd.File/Irccd.File.prototype.write.md
	js/module/Irccd.File/Irccd.File.remove.md
	js/module/Irccd.File/Irccd.File.stat.md
	js/module/Irccd/index.md
	js/module/Irccd.Logger/index.md
	js/module/Irccd.Logger/Irccd.Logger.debug.md
	js/module/Irccd.Logger/Irccd.Logger.info.md
	js/module/Irccd.Logger/Irccd.Logger.warning.md
	js/module/Irccd.Plugin/index.md
	js/module/Irccd.Plugin/Irccd.Plugin.info.md
	js/module/Irccd.Plugin/Irccd.Plugin.list.md
	js/module/Irccd.Plugin/Irccd.Plugin.load.md
	js/module/Irccd.Plugin/Irccd.Plugin.reload.md
	js/module/Irccd.Plugin/Irccd.Plugin.unload.md
	js/module/Irccd.Server/index.md
	js/module/Irccd.Server/Irccd.Server.add.md
	js/module/Irccd.Server/Irccd.Server.find.md
	js/module/Irccd.Server/Irccd.Server.list.md
	js/module/Irccd.Server/Irccd.Server.prototype.constructor.md
	js/module/Irccd.Server/Irccd.Server.prototype.info.md
	js/module/Irccd.Server/Irccd.Server.prototype.invite.md
	js/module/Irccd.Server/Irccd.Server.prototype.isSelf.md
	js/module/Irccd.Server/Irccd.Server.prototype.join.md
	js/module/Irccd.Server/Irccd.Server.prototype.kick.md
	js/module/Irccd.Server/Irccd.Server.prototype.me.md
	js/module/Irccd.Server/Irccd.Server.prototype.message.md
	js/module/Irccd.Server/Irccd.Server.prototype.mode.md
	js/module/Irccd.Server/Irccd.Server.prototype.names.md
	js/module/Irccd.Server/Irccd.Server.prototype.nick.md
	js/module/Irccd.Server/Irccd.Server.prototype.notice.md
	js/module/Irccd.Server/Irccd.Server.prototype.part.md
	js/module/Irccd.Server/Irccd.Server.prototype.topic.md
	js/module/Irccd.Server/Irccd.Server.prototype.toString.md
	js/module/Irccd.Server/Irccd.Server.prototype.whois.md
	js/module/Irccd.Server/Irccd.Server.remove.md
	js/module/Irccd.System/index.md
	js/module/Irccd.System/Irccd.System.env.md
	js/module/Irccd.System/Irccd.System.exec.md
	js/module/Irccd.System/Irccd.System.home.md
	js/module/Irccd.System/Irccd.System.name.md
	js/module/Irccd.System/Irccd.System.popen.md
	js/module/Irccd.System/Irccd.System.sleep.md
	js/module/Irccd.System/Irccd.System.ticks.md
	js/module/Irccd.System/Irccd.System.uptime.md
	js/module/Irccd.System/Irccd.System.usleep.md
	js/module/Irccd.System/Irccd.System.version.md
	js/module/Irccd.Timer/index.md
	js/module/Irccd.Timer/Irccd.Timer.prototype.constructor.md
	js/module/Irccd.Timer/Irccd.Timer.prototype.start.md
	js/module/Irccd.Timer/Irccd.Timer.prototype.stop.md
	js/module/Irccd.Unicode/index.md
	js/module/Irccd.Unicode/Irccd.Unicode.isDigit.md
	js/module/Irccd.Unicode/Irccd.Unicode.isLetter.md
	js/module/Irccd.Unicode/Irccd.Unicode.isLower.md
	js/module/Irccd.Unicode/Irccd.Unicode.isSpace.md
	js/module/Irccd.Unicode/Irccd.Unicode.isTitle.md
	js/module/Irccd.Unicode/Irccd.Unicode.isUpper.md
	js/module/Irccd.Util/index.md
	js/module/Irccd.Util/Irccd.Util.cut.md
	js/module/Irccd.Util/Irccd.Util.format.md
	js/module/Irccd.Util/Irccd.Util.splithost.md
	js/module/Irccd.Util/Irccd.Util.splituser.md
)

set(
	HTML_SOURCES
	build.md
	irccd.conf.md
	irccdctl.conf.md
	irccdctl.md
	irccd.md
	irccd-test.md
)

foreach (file ${HTML_SOURCES} ${HTML_JS_SOURCES})
	get_filename_component(directory ${file} DIRECTORY)

	if (${file} IN_LIST HTML_SOURCES)
		set(template ${html_SOURCE_DIR}/template.html)
	else ()
		set(template ${html_SOURCE_DIR}/template-js.html)
	endif ()

	irccd_build_html(
		SOURCE ${doc_SOURCE_DIR}/src/${file}
		OUTPUT_DIR ${directory}
		TEMPLATE ${template}
		OUTPUT_VAR output
		COMPONENT html
	)

	list(APPEND SOURCES ${doc_SOURCE_DIR}/src/${file})
	list(APPEND OUTPUTS ${output})
endforeach ()

set(
	CSS
	${html_SOURCE_DIR}/css/github-gist.css
	${html_SOURCE_DIR}/css/bulma.min.css
)

set(
	JS
	${html_SOURCE_DIR}/js/highlight.min.js
)

add_custom_target(
	doc-html ALL
	COMMAND
		${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/html
	COMMAND
		${CMAKE_COMMAND} -E copy ${CSS} ${JS} ${CMAKE_BINARY_DIR}/html
	SOURCES
		${OUTPUTS}
		${SOURCES}
		${html_SOURCE_DIR}/template.html
)

set_target_properties(doc-html PROPERTIES FOLDER "doc")

# Install resources files.
install(
	FILES ${CSS} ${JS}
	COMPONENT html
	DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

setg(CPACK_COMPONENT_HTML_DISPLAY_NAME "Documentation")
setg(CPACK_COMPONENT_HTML_DESCRIPTION "User guide and JavaScript API.")
setg(CPACK_COMPONENT_HTML_GROUP "HTML Documentation")
