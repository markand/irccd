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

project(html)

set(
    HTML_SOURCES
    ${html_SOURCE_DIR}/index.md
    ${html_SOURCE_DIR}/api/module/index.md
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
    ${html_SOURCE_DIR}/api/module/Irccd.Util/Irccd.Util.splithost.md
    ${html_SOURCE_DIR}/api/module/Irccd.Util/Irccd.Util.format.md
    ${html_SOURCE_DIR}/api/module/Irccd.Util/Irccd.Util.splituser.md
    ${html_SOURCE_DIR}/api/index.md
    ${html_SOURCE_DIR}/api/event/index.md
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
    ${html_SOURCE_DIR}/irccdctl/commands.md
    ${html_SOURCE_DIR}/irccdctl/configuring.md
    ${html_SOURCE_DIR}/irccdctl/usage.md
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

#
# For each files, define the following variables:
#
# - baseurl: relative path to get climb to the root documentation (e.g. ../../)
# - inputbase: file name with it's base directory (e.g. api/event/onMessage.md)
# - inputname: file name without it's directory (e.g. onMessage.md)
# - outputbase: file name, directory and html extension
#   (e.g api/event/onMessage.html)
#
if (WITH_HTML)
    foreach (file ${HTML_SOURCES})
        # 'baseurl': compute the tree from the file to this directory.
        # 'inputbase': the canonical file name.
        get_filename_component(fulldirectory ${file} DIRECTORY)
        get_filename_component(inputname ${file} NAME_WE)
        file(RELATIVE_PATH baseurl ${fulldirectory} ${html_SOURCE_DIR})

        # 'inputbase': relative to this source directory.
        file(RELATIVE_PATH inputbase ${html_SOURCE_DIR} ${file})

        # 'outputbase': replace .md to .html as file extension.
        string(REGEX REPLACE "^(.*)\\.md$" "\\1.html" outputbase ${inputbase})

        # If file is located at root, replace empty baseurl with "./".
        if (baseurl STREQUAL "")
            set(baseurl "./")
        endif ()

        # Configure the file so it can resolve CMake variables.
        configure_file(
            ${file}
            ${html_BINARY_DIR}/${inputbase}
        )

        # Create a list of parents for the breadcrumb widget.
        get_filename_component(parentlist ${inputbase} DIRECTORY)
        string(REPLACE "/" ";" parentlist "${parentlist}")
        set(parents "  -\n")
        set(parents "${parents}    name: \"index\"\n")
        set(parents "${parents}    path: \"${baseurl}index.html\"\n")

        set(path "${baseurl}")
        foreach (p ${parentlist})
            set(path "${path}${p}/")
            set(parents "${parents}  -\n")
            set(parents "${parents}    name: \"${p}\"\n")
            set(parents "${parents}    path: \"${path}index.html\"\n")
        endforeach ()

        configure_file(
            ${html_SOURCE_DIR}/resources/metadata.yml
            ${html_BINARY_DIR}/${inputbase}.yml
        )

        # Create an output target to that file.
        pandoc(
            OUTPUT ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/${outputbase}
            SOURCES
                ${html_BINARY_DIR}/${inputbase}.yml
                ${html_BINARY_DIR}/${inputbase}
            DEPENDS ${file}
            TEMPLATE ${html_SOURCE_DIR}/resources/template.html
            VARIABLE baseurl:${baseurl}
            FROM markdown
            TO html5
            STANDALONE TOC MAKE_DIRECTORY
        )

        list(APPEND OUTPUTS ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/${outputbase})
        install(
            FILES ${IRCCD_FAKEROOTDIR}/${WITH_DOCDIR}/${outputbase}
            COMPONENT docs
            DESTINATION ${WITH_DOCDIR}
            RENAME ${outputbase}
        )
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
