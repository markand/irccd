#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2017 David Demelier <markand@malikania.fr>
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

set(
    HTML_SOURCES
    ${html_SOURCE_DIR}/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Directory/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Directory/Irccd.Directory.remove.md
    ${html_SOURCE_DIR}/api/module/Irccd.Directory/Irccd.Directory.mkdir.md
    ${html_SOURCE_DIR}/api/module/Irccd.Directory/Irccd.Directory.find.md
    ${html_SOURCE_DIR}/api/module/Irccd.Directory/Irccd.Directory.prototype.constructor.md
    ${html_SOURCE_DIR}/api/module/Irccd.Directory/Irccd.Directory.prototype.remove.md
    ${html_SOURCE_DIR}/api/module/Irccd.Directory/Irccd.Directory.prototype.find.md
    ${html_SOURCE_DIR}/api/module/Irccd.ElapsedTimer/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.pause.md
    ${html_SOURCE_DIR}/api/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.elapsed.md
    ${html_SOURCE_DIR}/api/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.constructor.md
    ${html_SOURCE_DIR}/api/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.restart.md
    ${html_SOURCE_DIR}/api/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.reset.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.stat.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.dirname.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.exists.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.remove.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.basename.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.prototype.read.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.prototype.seek.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.prototype.stat.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.prototype.constructor.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.prototype.tell.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.prototype.dirname.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.prototype.readline.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.prototype.write.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.prototype.lines.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.prototype.close.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.prototype.remove.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/Irccd.File.prototype.basename.md
    ${html_SOURCE_DIR}/api/module/Irccd.Logger/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Logger/Irccd.Logger.warning.md
    ${html_SOURCE_DIR}/api/module/Irccd.Logger/Irccd.Logger.info.md
    ${html_SOURCE_DIR}/api/module/Irccd.Logger/Irccd.Logger.debug.md
    ${html_SOURCE_DIR}/api/module/Irccd.Plugin/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Plugin/Irccd.Plugin.unload.md
    ${html_SOURCE_DIR}/api/module/Irccd.Plugin/Irccd.Plugin.load.md
    ${html_SOURCE_DIR}/api/module/Irccd.Plugin/Irccd.Plugin.reload.md
    ${html_SOURCE_DIR}/api/module/Irccd.Plugin/Irccd.Plugin.info.md
    ${html_SOURCE_DIR}/api/module/Irccd.Plugin/Irccd.Plugin.list.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.remove.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.list.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.find.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.add.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.me.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.cmode.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.mode.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.part.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.message.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.topic.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.whois.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.nick.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.constructor.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.join.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.invite.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.cnotice.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.info.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.notice.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.kick.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.toString.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/Irccd.Server.prototype.names.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/Irccd.System.exec.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/Irccd.System.sleep.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/Irccd.System.popen.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/Irccd.System.env.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/Irccd.System.home.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/Irccd.System.uptime.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/Irccd.System.version.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/Irccd.System.usleep.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/Irccd.System.name.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/Irccd.System.ticks.md
    ${html_SOURCE_DIR}/api/module/Irccd.Timer/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Timer/Irccd.Timer.prototype.start.md
    ${html_SOURCE_DIR}/api/module/Irccd.Timer/Irccd.Timer.prototype.constructor.md
    ${html_SOURCE_DIR}/api/module/Irccd.Timer/Irccd.Timer.prototype.stop.md
    ${html_SOURCE_DIR}/api/module/Irccd/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Unicode/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Unicode/Irccd.Unicode.isSpace.md
    ${html_SOURCE_DIR}/api/module/Irccd.Unicode/Irccd.Unicode.isTitle.md
    ${html_SOURCE_DIR}/api/module/Irccd.Unicode/Irccd.Unicode.isUpper.md
    ${html_SOURCE_DIR}/api/module/Irccd.Unicode/Irccd.Unicode.isLetter.md
    ${html_SOURCE_DIR}/api/module/Irccd.Unicode/Irccd.Unicode.isDigit.md
    ${html_SOURCE_DIR}/api/module/Irccd.Unicode/Irccd.Unicode.isLower.md
    ${html_SOURCE_DIR}/api/module/Irccd.Util/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Util/Irccd.Util.cut.md
    ${html_SOURCE_DIR}/api/module/Irccd.Util/Irccd.Util.format.md
    ${html_SOURCE_DIR}/api/module/Irccd.Util/Irccd.Util.splithost.md
    ${html_SOURCE_DIR}/api/module/Irccd.Util/Irccd.Util.splituser.md
    ${html_SOURCE_DIR}/api/index.md
    ${html_SOURCE_DIR}/api/event/onWhois.md
    ${html_SOURCE_DIR}/api/event/onMessage.md
    ${html_SOURCE_DIR}/api/event/onPart.md
    ${html_SOURCE_DIR}/api/event/onQueryCommand.md
    ${html_SOURCE_DIR}/api/event/onMode.md
    ${html_SOURCE_DIR}/api/event/onNotice.md
    ${html_SOURCE_DIR}/api/event/onLoad.md
    ${html_SOURCE_DIR}/api/event/onInvite.md
    ${html_SOURCE_DIR}/api/event/onChannelNotice.md
    ${html_SOURCE_DIR}/api/event/onCommand.md
    ${html_SOURCE_DIR}/api/event/onKick.md
    ${html_SOURCE_DIR}/api/event/onReload.md
    ${html_SOURCE_DIR}/api/event/onChannelMode.md
    ${html_SOURCE_DIR}/api/event/onTopic.md
    ${html_SOURCE_DIR}/api/event/onConnect.md
    ${html_SOURCE_DIR}/api/event/onJoin.md
    ${html_SOURCE_DIR}/api/event/onMe.md
    ${html_SOURCE_DIR}/api/event/onNick.md
    ${html_SOURCE_DIR}/api/event/onNames.md
    ${html_SOURCE_DIR}/api/event/onUnload.md
    ${html_SOURCE_DIR}/api/event/onQuery.md
    ${html_SOURCE_DIR}/build/build-from-sources.md
    ${html_SOURCE_DIR}/build/build-options.md
    ${html_SOURCE_DIR}/dev/plugin-javascript-introduction.md
    ${html_SOURCE_DIR}/dev/socket-commands.md
    ${html_SOURCE_DIR}/dev/socket-protocol.md
    ${html_SOURCE_DIR}/irccd/configuring.md
    ${html_SOURCE_DIR}/irccd/paths.md
    ${html_SOURCE_DIR}/irccd/usage.md
    ${html_SOURCE_DIR}/irccdctl/configuring.md
    ${html_SOURCE_DIR}/irccdctl/usage.md
    ${html_SOURCE_DIR}/irccdctl/command/index.md
    ${html_SOURCE_DIR}/irccdctl/command/help.md
    ${html_SOURCE_DIR}/irccdctl/command/plugin-config.md
    ${html_SOURCE_DIR}/irccdctl/command/plugin-info.md
    ${html_SOURCE_DIR}/irccdctl/command/plugin-list.md
    ${html_SOURCE_DIR}/irccdctl/command/plugin-load.md
    ${html_SOURCE_DIR}/irccdctl/command/plugin-reload.md
    ${html_SOURCE_DIR}/irccdctl/command/plugin-unload.md
    ${html_SOURCE_DIR}/irccdctl/command/rule-add.md
    ${html_SOURCE_DIR}/irccdctl/command/rule-info.md
    ${html_SOURCE_DIR}/irccdctl/command/rule-list.md
    ${html_SOURCE_DIR}/irccdctl/command/rule-move.md
    ${html_SOURCE_DIR}/irccdctl/command/rule-remove.md
    ${html_SOURCE_DIR}/irccdctl/command/server-cmode.md
    ${html_SOURCE_DIR}/irccdctl/command/server-cnotice.md
    ${html_SOURCE_DIR}/irccdctl/command/server-connect.md
    ${html_SOURCE_DIR}/irccdctl/command/server-disconnect.md
    ${html_SOURCE_DIR}/irccdctl/command/server-invite.md
    ${html_SOURCE_DIR}/irccdctl/command/server-join.md
    ${html_SOURCE_DIR}/irccdctl/command/server-kick.md
    ${html_SOURCE_DIR}/irccdctl/command/server-list.md
    ${html_SOURCE_DIR}/irccdctl/command/server-me.md
    ${html_SOURCE_DIR}/irccdctl/command/server-message.md
    ${html_SOURCE_DIR}/irccdctl/command/server-mode.md
    ${html_SOURCE_DIR}/irccdctl/command/server-nick.md
    ${html_SOURCE_DIR}/irccdctl/command/server-notice.md
    ${html_SOURCE_DIR}/irccdctl/command/server-part.md
    ${html_SOURCE_DIR}/irccdctl/command/server-reconnect.md
    ${html_SOURCE_DIR}/irccdctl/command/server-topic.md
    ${html_SOURCE_DIR}/irccdctl/command/watch.md
    ${html_SOURCE_DIR}/misc/common-patterns-and-formatting.md
    ${html_SOURCE_DIR}/misc/configuration-syntax.md
)

set(
    CSS
    ${html_SOURCE_DIR}/resources/css/bootstrap.min.css
    ${html_SOURCE_DIR}/resources/css/doc-guide.css
    ${html_SOURCE_DIR}/resources/css/doc.css
    ${html_SOURCE_DIR}/resources/css/tomorrow.css
)

set(
    JS
    ${html_SOURCE_DIR}/resources/js/bootstrap.min.js
    ${html_SOURCE_DIR}/resources/js/highlight.min.js
    ${html_SOURCE_DIR}/resources/js/jquery.min.js
)

if (WITH_HTML)
    foreach (file ${HTML_SOURCES})
        irccd_build_html(
            SOURCE ${file}
            OUTPUT_VAR output
        )
        list(APPEND OUTPUTS ${output})

    endforeach ()

    add_custom_target(
        html ALL
        SOURCES
            ${HTML_SOURCES} ${CSS} ${JS} ${OUTPUTS}
            ${html_SOURCE_DIR}/resources/template.html
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/css
        COMMAND
            ${CMAKE_COMMAND} -E make_directory ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/js
        COMMAND
            ${CMAKE_COMMAND} -E copy ${CSS} ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/css
        COMMAND
            ${CMAKE_COMMAND} -E copy ${JS} ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/js
    )

    # Install resources files.
    install(
        FILES ${CSS}
        COMPONENT docs
        DESTINATION ${WITH_DOCDIR}/css
    )
    install(
        FILES ${JS}
        COMPONENT docs
        DESTINATION ${WITH_DOCDIR}/js
    )
endif ()

setg(CPACK_COMPONENT_DOCS_DISPLAY_NAME "Documentation")
setg(CPACK_COMPONENT_DOCS_DESCRIPTION "User guide and JavaScript API.")
setg(CPACK_COMPONENT_DOCS_GROUP "Documentation")
