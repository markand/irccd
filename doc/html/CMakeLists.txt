#
# CMakeLists.txt -- CMake build system for irccd
#
# Copyright (c) 2013-2018 David Demelier <markand@malikania.fr>
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
    HTML_API_SOURCES
    api/module/Irccd.Directory/index.md
    api/module/Irccd.Directory/Irccd.Directory.remove.md
    api/module/Irccd.Directory/Irccd.Directory.mkdir.md
    api/module/Irccd.Directory/Irccd.Directory.find.md
    api/module/Irccd.Directory/Irccd.Directory.prototype.constructor.md
    api/module/Irccd.Directory/Irccd.Directory.prototype.remove.md
    api/module/Irccd.Directory/Irccd.Directory.prototype.find.md
    api/module/Irccd.ElapsedTimer/index.md
    api/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.pause.md
    api/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.elapsed.md
    api/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.constructor.md
    api/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.restart.md
    api/module/Irccd.ElapsedTimer/Irccd.ElapsedTimer.prototype.reset.md
    api/module/Irccd.File/index.md
    api/module/Irccd.File/Irccd.File.stat.md
    api/module/Irccd.File/Irccd.File.dirname.md
    api/module/Irccd.File/Irccd.File.exists.md
    api/module/Irccd.File/Irccd.File.remove.md
    api/module/Irccd.File/Irccd.File.basename.md
    api/module/Irccd.File/Irccd.File.prototype.read.md
    api/module/Irccd.File/Irccd.File.prototype.seek.md
    api/module/Irccd.File/Irccd.File.prototype.stat.md
    api/module/Irccd.File/Irccd.File.prototype.constructor.md
    api/module/Irccd.File/Irccd.File.prototype.tell.md
    api/module/Irccd.File/Irccd.File.prototype.dirname.md
    api/module/Irccd.File/Irccd.File.prototype.readline.md
    api/module/Irccd.File/Irccd.File.prototype.write.md
    api/module/Irccd.File/Irccd.File.prototype.lines.md
    api/module/Irccd.File/Irccd.File.prototype.close.md
    api/module/Irccd.File/Irccd.File.prototype.remove.md
    api/module/Irccd.File/Irccd.File.prototype.basename.md
    api/module/Irccd.Logger/index.md
    api/module/Irccd.Logger/Irccd.Logger.warning.md
    api/module/Irccd.Logger/Irccd.Logger.info.md
    api/module/Irccd.Logger/Irccd.Logger.debug.md
    api/module/Irccd.Plugin/index.md
    api/module/Irccd.Plugin/Irccd.Plugin.unload.md
    api/module/Irccd.Plugin/Irccd.Plugin.load.md
    api/module/Irccd.Plugin/Irccd.Plugin.reload.md
    api/module/Irccd.Plugin/Irccd.Plugin.info.md
    api/module/Irccd.Plugin/Irccd.Plugin.list.md
    api/module/Irccd.Server/index.md
    api/module/Irccd.Server/Irccd.Server.remove.md
    api/module/Irccd.Server/Irccd.Server.list.md
    api/module/Irccd.Server/Irccd.Server.find.md
    api/module/Irccd.Server/Irccd.Server.add.md
    api/module/Irccd.Server/Irccd.Server.prototype.me.md
    api/module/Irccd.Server/Irccd.Server.prototype.mode.md
    api/module/Irccd.Server/Irccd.Server.prototype.part.md
    api/module/Irccd.Server/Irccd.Server.prototype.message.md
    api/module/Irccd.Server/Irccd.Server.prototype.topic.md
    api/module/Irccd.Server/Irccd.Server.prototype.whois.md
    api/module/Irccd.Server/Irccd.Server.prototype.nick.md
    api/module/Irccd.Server/Irccd.Server.prototype.constructor.md
    api/module/Irccd.Server/Irccd.Server.prototype.join.md
    api/module/Irccd.Server/Irccd.Server.prototype.invite.md
    api/module/Irccd.Server/Irccd.Server.prototype.info.md
    api/module/Irccd.Server/Irccd.Server.prototype.isSelf.md
    api/module/Irccd.Server/Irccd.Server.prototype.notice.md
    api/module/Irccd.Server/Irccd.Server.prototype.kick.md
    api/module/Irccd.Server/Irccd.Server.prototype.toString.md
    api/module/Irccd.Server/Irccd.Server.prototype.names.md
    api/module/Irccd.System/index.md
    api/module/Irccd.System/Irccd.System.exec.md
    api/module/Irccd.System/Irccd.System.sleep.md
    api/module/Irccd.System/Irccd.System.popen.md
    api/module/Irccd.System/Irccd.System.env.md
    api/module/Irccd.System/Irccd.System.home.md
    api/module/Irccd.System/Irccd.System.uptime.md
    api/module/Irccd.System/Irccd.System.version.md
    api/module/Irccd.System/Irccd.System.usleep.md
    api/module/Irccd.System/Irccd.System.name.md
    api/module/Irccd.System/Irccd.System.ticks.md
    api/module/Irccd.Timer/index.md
    api/module/Irccd.Timer/Irccd.Timer.prototype.start.md
    api/module/Irccd.Timer/Irccd.Timer.prototype.constructor.md
    api/module/Irccd.Timer/Irccd.Timer.prototype.stop.md
    api/module/Irccd/index.md
    api/module/Irccd.Unicode/index.md
    api/module/Irccd.Unicode/Irccd.Unicode.isSpace.md
    api/module/Irccd.Unicode/Irccd.Unicode.isTitle.md
    api/module/Irccd.Unicode/Irccd.Unicode.isUpper.md
    api/module/Irccd.Unicode/Irccd.Unicode.isLetter.md
    api/module/Irccd.Unicode/Irccd.Unicode.isDigit.md
    api/module/Irccd.Unicode/Irccd.Unicode.isLower.md
    api/module/Irccd.Util/index.md
    api/module/Irccd.Util/Irccd.Util.cut.md
    api/module/Irccd.Util/Irccd.Util.format.md
    api/module/Irccd.Util/Irccd.Util.splithost.md
    api/module/Irccd.Util/Irccd.Util.splituser.md
    api/index.md
    api/event/onWhois.md
    api/event/onMessage.md
    api/event/onPart.md
    api/event/onMode.md
    api/event/onNotice.md
    api/event/onLoad.md
    api/event/onInvite.md
    api/event/onCommand.md
    api/event/onKick.md
    api/event/onReload.md
    api/event/onTopic.md
    api/event/onConnect.md
    api/event/onJoin.md
    api/event/onMe.md
    api/event/onNick.md
    api/event/onNames.md
    api/event/onUnload.md
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

set(
    CSS
    ${html_SOURCE_DIR}/css/bootstrap.min.css
    ${html_SOURCE_DIR}/css/doc-guide.css
    ${html_SOURCE_DIR}/css/doc.css
    ${html_SOURCE_DIR}/css/tomorrow.css
)

set(
    JS
    ${html_SOURCE_DIR}/js/bootstrap.min.js
    ${html_SOURCE_DIR}/js/highlight.min.js
    ${html_SOURCE_DIR}/js/jquery.min.js
)

foreach (fname ${HTML_API_SOURCES} ${HTML_SOURCES})
    # API sources require a js variable to be set.
    if (${fname} IN_LIST HTML_API_SOURCES)
        set(VARIABLES js:1)
    else ()
        set(VARIABLES guide:1)
    endif ()

    irccd_build_html(
        SOURCE ${doc_SOURCE_DIR}/src/${fname}
        OUTPUT ${fname}
        OUTPUT_VAR output
        COMPONENT docs
        VARIABLES ${VARIABLES}
    )

    list(APPEND SOURCES ${doc_SOURCE_DIR}/src/${fname})
    list(APPEND OUTPUTS ${output})
endforeach ()

add_custom_target(
    html ALL
    SOURCES
        ${CSS}
        ${JS}
        ${OUTPUTS}
        ${SOURCES}
        ${html_SOURCE_DIR}/template.html
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${html_BINARY_DIR}/css
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${html_BINARY_DIR}/js
    COMMAND
        ${CMAKE_COMMAND} -E copy ${CSS} ${html_BINARY_DIR}/css
    COMMAND
        ${CMAKE_COMMAND} -E copy ${JS} ${html_BINARY_DIR}/js
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

setg(CPACK_COMPONENT_DOCS_DISPLAY_NAME "Documentation")
setg(CPACK_COMPONENT_DOCS_DESCRIPTION "User guide and JavaScript API.")
setg(CPACK_COMPONENT_DOCS_GROUP "Documentation")
