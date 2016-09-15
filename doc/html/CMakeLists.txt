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
    ${html_SOURCE_DIR}/api/module/Irccd.File/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/function/stat.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/function/dirname.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/function/exists.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/function/remove.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/function/basename.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/method/read.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/method/seek.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/method/stat.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/method/constructor.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/method/tell.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/method/dirname.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/method/readline.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/method/write.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/method/lines.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/method/close.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/method/remove.md
    ${html_SOURCE_DIR}/api/module/Irccd.File/method/basename.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/function/exec.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/function/sleep.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/function/popen.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/function/env.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/function/home.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/function/uptime.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/function/version.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/function/usleep.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/function/name.md
    ${html_SOURCE_DIR}/api/module/Irccd.System/function/ticks.md
    ${html_SOURCE_DIR}/api/module/Irccd.Timer/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Timer/method/start.md
    ${html_SOURCE_DIR}/api/module/Irccd.Timer/method/constructor.md
    ${html_SOURCE_DIR}/api/module/Irccd.Timer/method/stop.md
    ${html_SOURCE_DIR}/api/module/Irccd/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Plugin/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Plugin/function/unload.md
    ${html_SOURCE_DIR}/api/module/Irccd.Plugin/function/load.md
    ${html_SOURCE_DIR}/api/module/Irccd.Plugin/function/reload.md
    ${html_SOURCE_DIR}/api/module/Irccd.Plugin/function/info.md
    ${html_SOURCE_DIR}/api/module/Irccd.Plugin/function/list.md
    ${html_SOURCE_DIR}/api/module/Irccd.Directory/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Directory/function/remove.md
    ${html_SOURCE_DIR}/api/module/Irccd.Directory/function/mkdir.md
    ${html_SOURCE_DIR}/api/module/Irccd.Directory/function/find.md
    ${html_SOURCE_DIR}/api/module/Irccd.Directory/method/constructor.md
    ${html_SOURCE_DIR}/api/module/Irccd.Directory/method/remove.md
    ${html_SOURCE_DIR}/api/module/Irccd.Directory/method/find.md
    ${html_SOURCE_DIR}/api/module/Irccd.Unicode/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Unicode/function/isSpace.md
    ${html_SOURCE_DIR}/api/module/Irccd.Unicode/function/isTitle.md
    ${html_SOURCE_DIR}/api/module/Irccd.Unicode/function/isUpper.md
    ${html_SOURCE_DIR}/api/module/Irccd.Unicode/function/isLetter.md
    ${html_SOURCE_DIR}/api/module/Irccd.Unicode/function/isDigit.md
    ${html_SOURCE_DIR}/api/module/Irccd.Unicode/function/isLower.md
    ${html_SOURCE_DIR}/api/module/Irccd.Util/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Util/function/splithost.md
    ${html_SOURCE_DIR}/api/module/Irccd.Util/function/format.md
    ${html_SOURCE_DIR}/api/module/Irccd.Util/function/splituser.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/function/remove.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/function/list.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/function/find.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/function/add.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/me.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/cmode.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/mode.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/part.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/message.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/topic.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/whois.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/nick.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/constructor.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/join.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/invite.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/cnotice.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/info.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/notice.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/kick.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/toString.md
    ${html_SOURCE_DIR}/api/module/Irccd.Server/method/names.md
    ${html_SOURCE_DIR}/api/module/Irccd.Logger/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.Logger/function/warning.md
    ${html_SOURCE_DIR}/api/module/Irccd.Logger/function/info.md
    ${html_SOURCE_DIR}/api/module/Irccd.Logger/function/debug.md
    ${html_SOURCE_DIR}/api/module/Irccd.ElapsedTimer/index.md
    ${html_SOURCE_DIR}/api/module/Irccd.ElapsedTimer/method/pause.md
    ${html_SOURCE_DIR}/api/module/Irccd.ElapsedTimer/method/elapsed.md
    ${html_SOURCE_DIR}/api/module/Irccd.ElapsedTimer/method/constructor.md
    ${html_SOURCE_DIR}/api/module/Irccd.ElapsedTimer/method/restart.md
    ${html_SOURCE_DIR}/api/module/Irccd.ElapsedTimer/method/reset.md
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
    ${html_SOURCE_DIR}/resources/js/highlight.js
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
